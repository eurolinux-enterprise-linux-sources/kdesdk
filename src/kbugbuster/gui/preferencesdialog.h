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
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <kpagedialog.h>

class QCheckBox;
class QRadioButton;
class QTreeWidget;
class QTreeWidgetItem;
class KIntNumInput;
class ServerListView;

class PreferencesDialog : public KPageDialog
{
    Q_OBJECT
  public:
    explicit PreferencesDialog( QWidget* parent = 0, const char* name = 0 );
    ~PreferencesDialog();

    void createServerItem( ServerListView *listView, const QString &name,
                           const QString &url, const QString &version );

  public:
    void readConfig();
    void writeConfig();

  signals:
    void configChanged();

  protected slots:
    void setDefaults();
    void slotApply();
    void slotOk();
    void slotCancel();

    void addServer();
    void editServer();
    void removeServer();

    void selectServer();

  protected:
    void setupServerPage();
    void setupAdvancedPage();


  private:
    QCheckBox *mShowClosedCheckBox;
    QCheckBox *mShowWishesCheckBox;
    QCheckBox *mShowVotedCheckBox;
    QCheckBox *mSendBccCheckBox;
    KIntNumInput *mMinVotesInput;
    QRadioButton *mKMailButton;
    QRadioButton *mDirectButton;
    QRadioButton *mSendmailButton;
    QTreeWidget *mServerList;
};

class ServerListView;
class ServerItem;

class SelectServerDlg : public KDialog
{
    Q_OBJECT
public:
    SelectServerDlg(PreferencesDialog *parent );
    ServerItem *serverSelected();
protected slots:
    void slotDoubleClicked ( QTreeWidgetItem *, int );

protected:
    ServerListView *list;
};


#endif
