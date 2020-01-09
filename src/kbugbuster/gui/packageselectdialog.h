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
#ifndef PACKAGESELECTDIALOG_H
#define PACKAGESELECTDIALOG_H

#include <QtCore/QHash>
#include <QtGui/QKeyEvent>
#include <Q3ListView>

#include <kdialog.h>

#include "package.h"

class KCompletion;
class KLineEdit;
class QTreeWidgetItem;
class QTreeWidget;

class PackageListView : public Q3ListView
{
    Q_OBJECT
  public:
    PackageListView( QWidget *parent );

    void resetTyped();

  signals:
    void typed( const QString & );

  protected:
    void keyPressEvent( QKeyEvent *e );
    
  private:
    QString mTyped;
};

class PackageSelectDialog : public KDialog
{
    Q_OBJECT
  public:
    PackageSelectDialog(QWidget *parent=0);
    ~PackageSelectDialog();

    void setRecentPackages( const QStringList & );
    void setPackages( const Package::List &pkgs );

    Package selectedPackage();
    QString selectedComponent();

  protected slots:
    void slotOk();

  private slots:
    void recentSelected(QTreeWidgetItem *, int );
    void completeSelected( int, Q3ListViewItem * );
    void completeTyped( const QString & );

  private:
    Package::List mPackages;
    Package mSelectedPackage;
    QString mSelectedComponent;
    
    QTreeWidget *mRecentList;
    PackageListView *mCompleteList;
    KLineEdit *mPackageEdit;
    KCompletion *mCompletion;
    QHash<QString,Q3ListViewItem*> mCompletionDict;
};

#endif
