/*
    cwbugdetailscontainer.cpp  -  Container for bug details

    copyright   : (c) 2001 by Martijn Klingens
    email       : klingens@kde.org

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "cwbugdetailscontainer.h"

#include <QPushButton>
#include <QLayout>
#include <QtCore/QList>

#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>

#include "bugsystem.h"
#include "bugcommand.h"
#include "bugserver.h"

#include "cwbugdetails.h"
#include "cwloadingwidget.h"

#include <kstringhandler.h>

using namespace KBugBusterMainWindow;

CWBugDetailsContainer::CWBugDetailsContainer( QWidget *parent  )
: CWBugDetailsContainer_Base( parent )
{
    // Do some stuff Designer can't do:
    m_bugCloseBtn->setIcon( KIcon( "user-trash" ) );
    m_bugCloseSilentlyBtn->setIcon( KIcon( "user-trash" ) );
    m_bugReopenBtn->setIcon( KIcon( "help-hint" ) );
    m_bugReassignBtn->setIcon( KIcon( "folder-new" ) );
    m_bugTitleBtn->setIcon( KIcon( "edit-rename" ) );
    m_bugSeverityBtn->setIcon( KIcon( "document-properties" ) );
    m_bugReplyBtn->setIcon( KIcon( "mail-reply-all" ) );
    m_bugReplyPrivBtn->setIcon( KIcon( "mail-reply-sender" ) );

    // The Bugzilla mail interface doesn't support all commands yet.
    m_bugCloseSilentlyBtn->hide();
    m_bugReassignBtn->hide();
    m_bugTitleBtn->hide();
    m_bugSeverityBtn->hide();

    // Create Bug Details pane
    m_bugDetails = new CWBugDetails( m_bugStack );

    // Fill WidgetStack in Bug Details pane
    m_bugLoading = new CWLoadingWidget( CWLoadingWidget::BottomFrame,
                                        m_bugStack );
    connect( m_bugLoading, SIGNAL( clicked() ), SIGNAL( searchBugNumber() ) );

    m_bugStack->insertWidget( 0,m_bugDetails );
    m_bugStack->insertWidget( 1,m_bugLoading );

    setNoBug();

    QFont f = m_bugLabel->font();
    f.setBold( true );
    m_bugLabel->setFont( f );

    // Set fonts and margins
    //CWBugDetailsContainer_BaseLayout->setSpacing( KDialog::spacingHint() );
    //CWBugDetailsContainer_BaseLayout->setMargin( KDialog::marginHint() );

    connect( m_bugCloseBtn, SIGNAL( clicked() ), SIGNAL( signalCloseBug() ) );
    connect( m_bugCloseSilentlyBtn, SIGNAL( clicked() ), SIGNAL( signalCloseBugSilently() ) );
    connect( m_bugReopenBtn, SIGNAL( clicked() ), SIGNAL( signalReopenBug() ) );
    connect( m_bugReassignBtn, SIGNAL( clicked() ), SIGNAL( signalReassignBug() ) );
    connect( m_bugTitleBtn, SIGNAL( clicked() ), SIGNAL( signalTitleBug() ) );
    connect( m_bugSeverityBtn, SIGNAL( clicked() ), SIGNAL( signalSeverityBug() ) );
    connect( m_bugReplyBtn, SIGNAL( clicked() ), SIGNAL( signalReplyBug() ) );
    connect( m_bugReplyPrivBtn, SIGNAL( clicked() ), SIGNAL( signalReplyPrivateBug() ) );

    connect( m_cmdClearBtn, SIGNAL( clicked() ), SIGNAL( signalClearCommand() ) );

    connect( BugSystem::self(), SIGNAL( bugDetailsLoading( const Bug & ) ),
             SLOT( setLoading( const Bug & ) ) );
    connect( BugSystem::self(), SIGNAL( bugDetailsCacheMiss( const Bug & ) ),
             SLOT( setCacheMiss( const Bug & ) ) );
    connect( BugSystem::self(), SIGNAL( commandQueued( BugCommand * ) ),
             SLOT( commandQueued( BugCommand * ) ) );
    connect( BugSystem::self(), SIGNAL( commandCanceled( const QString & ) ),
             SLOT( clearCommand( const QString & ) ) );
}

CWBugDetailsContainer::~CWBugDetailsContainer()
{
}

void CWBugDetailsContainer::setBug( const Bug &bug, const BugDetails &details )
{
    m_bug = bug;
    m_bugDetails->setBug( bug, details );

    QString labelText;

    if ( bug.mergedWith().size() )
    {
       //FIXME: What should the separator be for lists? Don't see anything in KLocale for that
       QString list;
       Bug::BugMergeList mergedWith = bug.mergedWith();
       for (Bug::BugMergeList::ConstIterator i = mergedWith.constBegin(); i != mergedWith.constEnd(); ++i)
       {
            list += QString::number(*i)+", ";
       }

       list.truncate( list.length()-2 ); //Strip off the last ", "

       labelText  = i18nc("bug #number [Merged with: a list of bugs] (severity): title","Bug #%1 [Merged with: %2] (%3): %4",
                               bug.number() ,
                               list ,
                               bug.severityAsString() ,
                               bug.title() );

    }
    else
    {
       labelText  = i18nc("bug #number (severity): title","Bug #%1 (%2): %3",
                               bug.number(), bug.severityAsString() ,
                               bug.title() );
    }

    m_bugLabel->setText( KStringHandler::tagUrls( labelText ) );

    showCommands( bug );

    enableButtons( bug );

    m_bugStack->setCurrentIndex( 0 );
    emit resetProgressBar();
}

void CWBugDetailsContainer::showCommands( const Bug& bug )
{
    QList<BugCommand*> commands = BugSystem::self()->server()->queryCommands( bug );
    if ( !commands.isEmpty() ) {
        QString cmdDetails;
        QString cmdText = i18n("Pending commands:")+' ';
        bool first = true;
        QListIterator<BugCommand*> cmdIt( commands );
        while( cmdIt.hasNext() )
        {
            BugCommand *cmd = cmdIt.next();
            if (!first)
                cmdText += "  |  "; // separator in case of multiple commands
            first = false;
            cmdText += QString("<b>%1</b>").arg( cmd->name() );
            if (!cmdDetails.isEmpty())
                cmdDetails += "  |  ";  // separator in case of multiple commands
            cmdDetails += cmd->details();
        }
        // Set summary as text label, details into tooltip
        m_cmdLabel->setText( cmdText );
        if ( !cmdDetails.isEmpty() ) {
            m_cmdLabel->setToolTip( cmdDetails );
        } else {
            m_cmdLabel->setToolTip("");
        }
        m_cmdLabel->show();
    } else {
        hideCommands();
    }
}

void CWBugDetailsContainer::hideCommands()
{
    m_cmdLabel->hide();
}

void CWBugDetailsContainer::clearCommand( const QString &bug )
{
    if ( bug == m_bug.number() )
        showCommands( m_bug );
}

void CWBugDetailsContainer::commandQueued( BugCommand *cmd )
{
    // ### use == operator instead?
    // (might not work because impl is different)
    if ( cmd && cmd->bug().number() == m_bug.number() )
        showCommands( m_bug );
}

void CWBugDetailsContainer::setNoBug()
{
    m_bugLabel->setText( i18n("Bug Title") );

    m_bug = Bug();
    showCommands( m_bug );

    m_bugLoading->setText( i18n( "Click here to select a bug by number" ) );
    m_bugStack->setCurrentIndex( 1 );
}

void CWBugDetailsContainer::setLoading( const Bug &bug )
{
    m_bug = bug;
    showCommands( bug );

    m_bugLoading->setText(i18n( "Retrieving Details for Bug %1\n\n(%2)" ,
                            bug.number(), bug.title() ) );
    m_bugStack->setCurrentIndex( 1 );
}

void CWBugDetailsContainer::setCacheMiss( const Bug &bug )
{
    m_bug = bug;
    showCommands( bug );

    QString msg;
    if( BugSystem::self()->disconnected() )
        msg = i18n( "Bug #%1 (%2) is not available offline." , 
               bug.number(), bug.title() );
    else
        msg = i18n( "Retrieving details for bug #%1\n"
                    "(%2)" , 
               bug.number(), bug.title() );
    m_bugLoading->setText( msg );
    m_bugStack->setCurrentIndex( 1 );
}


void CWBugDetailsContainer::enableButtons( const Bug &bug )
{
    if( bug.isNull() ) {
        m_bugCloseBtn->setEnabled( false );
        m_bugCloseSilentlyBtn->setEnabled( false );
        m_bugReopenBtn->setEnabled( false );
        m_bugReassignBtn->setEnabled( false );
        m_bugTitleBtn->setEnabled( false );
        m_bugSeverityBtn->setEnabled( false );
        m_bugReplyBtn->setEnabled( false );
        m_bugReplyPrivBtn->setEnabled( false );
    } else {
        if( bug.status() != Bug::Closed ) {
            m_bugCloseBtn->setEnabled( true );
            m_bugCloseSilentlyBtn->setEnabled( true );
            m_bugReopenBtn->setEnabled( false );
        } else {
            m_bugCloseBtn->setEnabled( false );
            m_bugCloseSilentlyBtn->setEnabled( false );
            m_bugReopenBtn->setEnabled( true );
        }
        m_bugReassignBtn->setEnabled( true );
        m_bugTitleBtn->setEnabled( true );
        m_bugSeverityBtn->setEnabled( true );
        m_bugReplyBtn->setEnabled( true );
        m_bugReplyPrivBtn->setEnabled( true );
    }
}

#include "cwbugdetailscontainer.moc"

/* vim: set et ts=4 sw=4 softtabstop=4: */

