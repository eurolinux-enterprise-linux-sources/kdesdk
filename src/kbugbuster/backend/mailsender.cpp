/* This file is part of kdesdk / KBugBuster.

   Copyright 2008  KBugBuster Authors <kde-ev-board@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy 
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef QT_NO_ASCII_CAST
#define QT_NO_ASCII_CAST
#endif

#include "mailsender.h"

#include <unistd.h>
#include <stdio.h>

#include <QTimer>
#include <QApplication>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kshell.h>
#include <QtDBus>

#include "smtp.h"

MailSender::MailSender(MailClient client,const QString &smtpServer) :
  m_client( client ), m_smtpServer( smtpServer )
{
}

MailSender::~MailSender()
{
}

MailSender *MailSender::clone() const
{
  return new MailSender(m_client,m_smtpServer);
}

bool MailSender::send(const QString &fromName,const QString &fromEmail,const QString &to,
                      const QString &subject,const QString &body,bool bcc,
                      const QString &recipient)
{
  QString from( fromName );
  if ( !fromEmail.isEmpty() )
    from += QString::fromLatin1( " <%2>" ).arg( fromEmail );
  kDebug() << "MailSender::sendMail():\nFrom: " << from << "\nTo: " << to
            << "\nbccflag:" << bcc
            << "\nRecipient:" << recipient
            << "\nSubject: " << subject << "\nBody: \n" << body << endl;

  // ### FIXME: bcc is not supported in direct mode and recipient is not
  // supported in sendmail and kmail mode

  if (m_client == Sendmail) {
    kDebug() << "Sending per Sendmail";

    bool needHeaders = true;

    QString command = KStandardDirs::findExe(QString::fromLatin1("sendmail"),
        QString::fromLatin1("/sbin:/usr/sbin:/usr/lib"));
    if (!command.isNull()) command += QString::fromLatin1(" -oi -t");
    else {
      command = KStandardDirs::findExe(QString::fromLatin1("mail"));
      if (command.isNull()) return false; // give up

      command.append(QString::fromLatin1(" -s "));
      command.append(KShell::quoteArg(subject));

      if (bcc) {
        command.append(QString::fromLatin1(" -b "));
        command.append(KShell::quoteArg(from));
      }

      command.append(" ");
      command.append(KShell::quoteArg(to));

      needHeaders = false;
    }

    FILE * fd = popen(command.toLocal8Bit(),"w");
    if (!fd)
    {
      kError() << "Unable to open a pipe to " << command << endl;
      QTimer::singleShot( 0, this, SLOT( deleteLater() ) );
      return false;
    }

    QString textComplete;
    if (needHeaders)
    {
      textComplete += QString::fromLatin1("From: ") + from + '\n';
      textComplete += QString::fromLatin1("To: ") + to + '\n';
      if (bcc) textComplete += QString::fromLatin1("Bcc: ") + from + '\n';
      textComplete += QString::fromLatin1("Subject: ") + subject + '\n';
      textComplete += QString::fromLatin1("X-Mailer: KBugBuster") + '\n';
    }
    textComplete += '\n'; // end of headers
    textComplete += body;

    emit status( i18n( "Sending through sendmail..." ) );
    fwrite(textComplete.toLocal8Bit(),textComplete.length(),1,fd);

    pclose(fd);
  } else if ( m_client == KMail ) {
    kDebug() << "Sending per KMail";

    if( !QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmail")) {
      KMessageBox::error(0,i18n("No running instance of KMail found."));
      QTimer::singleShot( 0, this, SLOT( deleteLater() ) );
      return false;
    }

    emit status( i18n( "Passing mail to KDE email program..." ) );
    if (!kMailOpenComposer(to,"", (bcc ? from : ""), subject,body,0,KUrl())) {
      QTimer::singleShot( 0, this, SLOT( deleteLater() ) );
      return false;
    }
  } else if ( m_client == Direct ) {
    kDebug() << "Sending Direct";

    QStringList recipients;
    if ( !recipient.isEmpty() )
        recipients << recipient;
    else
        recipients << to;

    QString message = QString::fromLatin1( "From: " ) + from +
                      QString::fromLatin1( "\nTo: " ) + to +
                      QString::fromLatin1( "\nSubject: " ) + subject +
                      QString::fromLatin1( "\nX-Mailer: KBugBuster" ) +
                      QString::fromLatin1( "\n\n" ) + body;

    Smtp *smtp = new Smtp( fromEmail, recipients, message, m_smtpServer );
    connect( smtp, SIGNAL( status( const QString & ) ),
             this, SIGNAL( status( const QString & ) ) );
    connect( smtp, SIGNAL( success() ),
             this, SLOT( smtpSuccess() ) );
    connect( smtp, SIGNAL( error( const QString &, const QString & ) ),
             this, SLOT( smtpError( const QString &, const QString & ) ) );

    this->setParent( smtp ); // die when smtp dies
  } else {
    kDebug() << "Invalid mail client setting.";
  }

  if (m_client != Direct)
  {
    emit finished();
    QTimer::singleShot( 0, this, SLOT( deleteLater() ) );
  }

  return true;
}

void MailSender::smtpSuccess()
{
  if ( parent() != sender() || !parent()->inherits( "Smtp" ) )
    return;

  static_cast<Smtp *>( parent() )->quit();
  emit finished();
}

void MailSender::smtpError(const QString &command, const QString &response)
{
  if ( parent() != sender() || !parent()->inherits( "Smtp" ) )
    return;

  Smtp *smtp = static_cast<Smtp *>( parent() );
  this->setParent( 0 );
  delete smtp;

  KMessageBox::error( qApp->activeWindow(),
                      i18n( "Error during SMTP transfer.\n"
                            "command: %1\n"
                            "response: %2", command, response ) );

  emit finished();
  QTimer::singleShot( 0, this, SLOT( deleteLater() ) );
}

int MailSender::kMailOpenComposer(const QString& arg0,const QString& arg1,
  const QString& arg2,const QString& arg3,const QString& arg4,int arg5,
  const KUrl& arg6)
{
  int result = 0;
#ifdef __GNUC__
#warning "kde4: test it"
#endif
  QDBusInterface kmail("org.kde.kmail", "/KMail", "org.kde.kmail.kmail");
  QDBusReply<int> reply = kmail.call("openComposer", arg0, arg1, arg2, arg3, arg4, arg5, arg6.url());
  if ( reply.isValid() )
  {
      result = reply;
  }
  else
      kDebug() << "kMailOpenComposer() call failed.";
  return result;
}

#include "mailsender.moc"

