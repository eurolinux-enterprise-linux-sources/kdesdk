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
#ifndef MSGINPUTDIALOG_H
#define MSGINPUTDIALOG_H

#include <kdialog.h>

#include "bug.h"
#include "package.h"

class KTextEdit;
class QSplitter;
class KListWidget;
class QListWidgetItem;
class KComboBox;

class MsgInputDialog : public KDialog
{
    Q_OBJECT
  public:
    enum MessageType{ Close, Reply, ReplyPrivate };
  
    MsgInputDialog( MessageType, const Bug &, const Package &,
                    const QString &, QWidget *parent=0);
    virtual ~MsgInputDialog();

    QString message() const;

  protected slots:
    void slotOk();
    void slotCancel();

  private slots:
    void editPresets();
    void updatePresets();
    void slotPresetSelected(QListWidgetItem * );
    void clearMessage();
    void queueCommand();
  
  private:
    void createButtons();
    void createLayout();
  
    void readConfig();
    void writeConfig();
  
    void insertQuotedMessage( const QString &quotedMsg );

    KComboBox *mRecipient;
    KTextEdit *mMessageEdit;
    QSplitter *mSplitter;
    KListWidget *mPresets;

    Bug mBug;
    Package mPackage;
    MessageType mType;
};

#endif
