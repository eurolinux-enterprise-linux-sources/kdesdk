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
#include "severityselectdialog.h"

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>

#include <kdebug.h>

#include "bugsystem.h"
#include "kbbprefs.h"

#include "severityselectdialog.moc"

SeveritySelectDialog::SeveritySelectDialog(QWidget *parent) :
  KDialog( parent )
{
    setCaption( i18n("Select Severity") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    mButtonGroup = new QGroupBox( i18n("Severity"), this );
    mButtonGroup->setAlignment( Qt::Horizontal );
    setMainWidget( mButtonGroup );

    QList<Bug::Severity> severities = Bug::severities();
    QList<Bug::Severity>::ConstIterator it;
    QVBoxLayout *vbox = new QVBoxLayout;
    for( it = severities.constBegin(); it != severities.constEnd(); ++it ) {
        QRadioButton *rb = new QRadioButton( Bug::severityToString( *it ), mButtonGroup );
        vbox->addWidget( rb );
        mSeverities.insert( *it, rb );
    }
    mButtonGroup->setLayout( vbox );
}

void SeveritySelectDialog::setSeverity( Bug::Severity s )
{
    if( mSeverities.contains( s ) )
        mSeverities[s]->setChecked( true );
}

Bug::Severity SeveritySelectDialog::selectedSeverity()
{
    QMapIterator<int, QRadioButton*> it(mSeverities);
    while( it.hasNext() ) {
        if(it.next().value()->isChecked())
            return (Bug::Severity)(it.key());
    }
    return (Bug::Severity)0;
}

QString SeveritySelectDialog::selectedSeverityAsString()
{
    return Bug::severityToString( selectedSeverity() );
}
