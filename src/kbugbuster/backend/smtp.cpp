/* This file is part of kdesdk / KBugBuster.

   Copyright 2008  Don Sanders <sanders@kde.org>

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
#include "smtp.h"

#include <QTextStream>
#include <QTcpSocket>
#include <QTimer>
#include <kmessagebox.h>
#include <klocale.h>

Smtp::Smtp( const QString &from, const QStringList &to,
	    const QString &aMessage,
	    const QString &server,
	    unsigned short int port )
{
    skipReadResponse = false;
    mSocket = new QTcpSocket( this );
    connect ( mSocket, SIGNAL( readyRead() ),
	      this, SLOT( readyRead() ) );
    connect ( mSocket, SIGNAL( connected() ),
	      this, SLOT( connected() ) );
    connect ( mSocket, SIGNAL( error(QAbstractSocket::SocketError) ),
	      this, SLOT( socketError(QAbstractSocket::SocketError) ) );

    message = aMessage;

    this->from = from;
    rcpt = to;
    state = smtpInit;
    command = "";

    emit status( i18n( "Connecting to %1", server ) );

    mSocket->connectToHost( server, port );
    t = new QTextStream( mSocket );
    t->setCodec( "ISO 8859-1" );
}


Smtp::~Smtp()
{
    delete t;
    delete mSocket;
}


void Smtp::send( const QString &from, const QStringList &to,
	    const QString &aMessage )
{
    skipReadResponse = true;
    message = aMessage;
    this->from = from;
    rcpt = to;

    state = smtpMail;
    command = "";
    readyRead();
}


void Smtp::quit()
{
    skipReadResponse = true;
    state = smtpQuit;
    command = "";
    readyRead();
}


void Smtp::connected()
{
    emit status( i18n( "Connected to %1", mSocket->peerName() ) );
}

void Smtp::socketError(QAbstractSocket::SocketError errorCode)
{
    command = "CONNECT";
    switch ( errorCode ) {
        case QAbstractSocket::ConnectionRefusedError:
	    responseLine = i18n( "Connection refused." );
	    break;
        case QAbstractSocket::HostNotFoundError:
	    responseLine = i18n( "Host Not Found." );
	    break;
        case QAbstractSocket::SocketAccessError:
	    responseLine = i18n( "Error reading socket." );
	    break;
        default:
	    responseLine = i18n( "Internal error, unrecognized error." );
    }
    QTimer::singleShot( 0, this, SLOT(emitError()) );
}

void Smtp::emitError() {
    error( command, responseLine );
}

void Smtp::readyRead()
{
    if (!skipReadResponse) {
	// SMTP is line-oriented
	if ( !mSocket->canReadLine() )
	    return;

	do {
	    responseLine = mSocket->readLine();
	    response += responseLine;
	} while( mSocket->canReadLine() && responseLine[3] != ' ' );
    }
    skipReadResponse = false;

    if ( state == smtpInit && responseLine[0] == '2' ) {
	// banner was okay, let's go on
	command = "HELO there";
	*t << "HELO there\r\n";
	state = smtpMail;
    } else if ( state == smtpMail && responseLine[0] == '2' ) {
	// HELO response was okay (well, it has to be)
	command = "MAIL";
	*t << "MAIL FROM: <" << from << ">\r\n";
	state = smtpRcpt;
    } else if ( state == smtpRcpt && responseLine[0] == '2' && (rcpt.begin() != rcpt.end())) {
	command = "RCPT";
	*t << "RCPT TO: <" << *(rcpt.begin()) << ">\r\n";
	rcpt.erase( rcpt.begin() );
	if (rcpt.begin() == rcpt.end())
	    state = smtpData;
    } else if ( state == smtpData && responseLine[0] == '2' ) {
	command = "DATA";
	*t << "DATA\r\n";
	state = smtpBody;
    } else if ( state == smtpBody && responseLine[0] == '3' ) {
	command = "DATA";
	QString separator = "";
	if (message[message.length() - 1] != '\n')
	    separator = "\r\n";
	*t << message << separator << ".\r\n";
	state = smtpSuccess;
    } else if ( state == smtpSuccess && responseLine[0] == '2' ) {
	QTimer::singleShot( 0, this, SIGNAL(success()) );
    } else if ( state == smtpQuit && responseLine[0] == '2' ) {
	command = "QUIT";
	*t << "QUIT\r\n";
	// here, we just close.
	state = smtpClose;
	emit status( i18n( "Message sent" ) );
    } else if ( state == smtpClose ) {
	// we ignore it
    } else { // error occurred
	QTimer::singleShot( 0, this, SLOT(emitError()) );
	state = smtpClose;
    }

    response = "";

    if ( state == smtpClose ) {
	delete t;
	t = 0;
	delete mSocket;
	mSocket = 0;
	QTimer::singleShot( 0, this, SLOT(deleteMe()) );
    }
}


void Smtp::deleteMe()
{
    delete this;
}

#include "smtp.moc"
