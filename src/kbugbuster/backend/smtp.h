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
#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTcpSocket>

class Smtp : public QObject
{
    Q_OBJECT

public:
    Smtp( const QString &from, const QStringList &to, const QString &message, 
          const QString &server, unsigned short int port = 25 );
    ~Smtp();
    void send( const QString &, const QStringList &, const QString & );
    void quit();


signals:
    void success();
    void status( const QString & );
    void error( const QString &command, const QString &response );

private slots:
    void readyRead();
    void connected();
    void deleteMe();
    void socketError(QAbstractSocket::SocketError error);
    void emitError();

private:
    enum State {
	smtpInit,
	smtpMail,
	smtpRcpt,
	smtpData,
	smtpBody,
	smtpSuccess,
	smtpQuit,
	smtpClose
    };

    QString message;
    QString from;
    QStringList rcpt;
    QTcpSocket *mSocket;
    QTextStream * t;
    int state;
    QString response, responseLine;
    bool skipReadResponse;
    QString command;
};

#endif
