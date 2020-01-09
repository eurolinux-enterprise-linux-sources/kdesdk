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
#ifndef MESSAGEEDITOR_H
#define MESSAGEEDITOR_H

#include <kdialog.h>

class KComboBox;
class KTextEdit;

class MessageEditor : public KDialog {
    Q_OBJECT
  public:
    MessageEditor( QWidget *parent );

  protected slots:
    void slotOk();

  private slots:
    void addButton();
    void removeButton();
    void changeMessage();
    void saveMessage();
    void updateMessage();
    void updateConfig();
    
  private:
    KComboBox *mSelectionCombo;
    KTextEdit *mMessageEdit;
    
    QString mCurrentButton;
    QMap <QString,QString> mMessageButtons;
};

#endif
