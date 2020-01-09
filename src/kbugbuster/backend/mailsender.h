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
#ifndef MAILSENDER_H
#define MAILSENDER_H

#include <QString>
#include <QObject>

class KUrl;

class MailSender : public QObject
{
  Q_OBJECT
  public:
    enum MailClient { Sendmail = 0, KMail = 1, Direct = 2 };
  
    explicit MailSender(MailClient,const QString &smtpServer=QString());
    virtual ~MailSender();

    MailSender *clone() const;

    /**
      Send mail with specified from, to and subject field and body as text. If
      bcc is set, send a blind carbon copy to the sender from.
      If recipient is specified the mail is sent to the specified address
      instead of 'to' . (this currently only works in for direct mail
      sending through SMTP.
    */
    bool send(const QString &fromName, const QString &fromEmail,
              const QString &to,const QString &subject,
              const QString &body,bool bcc=false, 
              const QString &recipient = QString());

  signals:
    void status( const QString &message );
    void finished();

  private slots:
    void smtpSuccess();
    void smtpError(const QString &command, const QString &response);

  private:
    int kMailOpenComposer(const QString& arg0,const QString& arg1,
                          const QString& arg2,const QString& arg3,
                          const QString& arg4,int arg5,const KUrl& arg6);

    MailClient m_client;
    QString m_smtpServer;
};

#endif
