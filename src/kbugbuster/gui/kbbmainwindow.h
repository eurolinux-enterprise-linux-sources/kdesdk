/*
    kbbmainwindow.h - KBugBuster's main window

    Copyright (c) 2001-2004 by Martijn Klingens      <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KBBMAINWINDOW_H
#define KBBMAINWINDOW_H

#include <kbookmarkmanager.h>
#include <kxmlguiwindow.h>
#include <qmap.h>
//Added by qt3to4:
#include <QLabel>

#include "package.h"
#include "bug.h"
#include "bugdetails.h"

class QAction;
class KActionMenu;
class KAction;
class KBookmarkMenu;
class KToggleAction;
class KSelectAction;
class QLabel;
class QProgressBar;
class PreferencesDialog;
class TextViewer;
class PackageSelectDialog;

namespace KBugBusterMainWindow
{
    class CentralWidget;
}

/**
 * @author Martijn Klingens
 */
class KBBMainWindow : public KXmlGuiWindow, virtual public KBookmarkOwner
{
    Q_OBJECT
  public:
    /**
     * construtor of KBugBusterApp, calls all init functions to create the application.
     */
    explicit KBBMainWindow( const QByteArray &initialPackage = "",
			    const QByteArray &initialCpomponent = "",
    			    const QByteArray &initialBug = "",
    			    QWidget* parent = 0, const char* name = 0 );
    ~KBBMainWindow();

    /// Overloaded functions of KBookmarkOwner
    virtual void openBookmark(const KBookmark & bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km);
    virtual QString currentTitle() const;
    virtual QString currentUrl() const;

  public slots:
    /**
     * Event handlers for our KActions
     */
    void slotStatusMsg( const QString &text );
    void slotDisconnectedAction();
    void slotSubmit();
    void slotListChanges();
    void slotSetPercent( unsigned long percent );
    void slotSelectServer();

    void showLastResponse();
    void showBugSource();

    void clearCache();

    /**
     * Other event handlers
     */

    void searchPackage();
    void searchBugNumber();
    void searchDescription();

    void preferences();
    void updatePackage();
    void slotToggleDone();
    void slotToggleWishes();

  protected:
    virtual bool queryClose();

  protected slots:
    void setupSelectServerAction();
    void slotToggleMenubar();

  private:
    void initActions();

    /**
     * Our main widget
     */
    KBugBusterMainWindow::CentralWidget *m_mainWidget;

    /**
     * Used KActions
     */
    QAction *fileQuit;
    KAction *reloadpacklist;
    KAction *reloadpack;
    KAction *reloadbug;
    KAction *reloadall;
    QAction *loadMyBugs;
    KToggleAction *m_disconnectedAction;

    /**
     * Status bar label. We need this, because the default Qt version doesn't
     * support rich text in the messages
     */
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;

    PreferencesDialog *mPreferencesDialog;

    KActionMenu *m_pamBookmarks;
    KBookmarkMenu* m_pBookmarkMenu;

    KSelectAction *mSelectServerAction;

    TextViewer *mResponseViewer;
    TextViewer *mBugSourceViewer;

    PackageSelectDialog *mPackageSelectDialog;
};

#endif

/* vim: set et ts=4 softtabstop=4 sw=4: */

