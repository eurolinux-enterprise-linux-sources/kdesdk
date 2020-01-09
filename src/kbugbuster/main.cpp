/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : zo mrt 18 17:12:24 CET 2001
    copyright            : (C) 2001 by Martijn Klingens
    email                : klingens@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>
#include "version.h"
#include "gui/kbbmainwindow.h"
#include "bugsystem.h"
#include "kbbprefs.h"

static const char description[] =
    I18N_NOOP("KBugBuster");

int main(int argc, char *argv[])
{
    KAboutData aboutData( "kbugbuster", 0, ki18n( "KBugBuster" ),
        KBUGBUSTER_VERSION, ki18n(description), KAboutData::License_GPL,
        ki18n("(c) 2001,2002,2003 the KBugBuster authors") );
    aboutData.addAuthor( ki18n("Martijn Klingens"), KLocalizedString(), "klingens@kde.org" );
    aboutData.addAuthor( ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org" );
    aboutData.addAuthor( ki18n("Simon Hausmann"), KLocalizedString(), "hausmann@kde.org" );
    aboutData.addAuthor( ki18n("David Faure"), KLocalizedString(), "faure@kde.org" );
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("d");
    options.add("disconnected", ki18n("Start in disconnected mode"));
    options.add("pkg");
    options.add("package <pkg>", ki18n("Start with the buglist for <package>"));
    options.add("bug <br>", ki18n("Start with bug report <br>"));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("disconnected")) {
        BugSystem::self()->setDisconnected( true );
    }

    if ( app.isSessionRestored() )
    {
        RESTORE( KBBMainWindow );
    }
    else 
    {
        KBBMainWindow *mainWin = new KBBMainWindow(args->getOption("package").toLocal8Bit(), args->getOption("bug").toLocal8Bit());

        // Since all background jobs remain running after closing the
        // main window we force a quit here
        QObject::connect( &app, SIGNAL( lastWindowClosed() ),
                          &app, SLOT( quit() ) );
        mainWin->show();
        return app.exec();
    }
}

/* vim: set et ts=4 sw=4 softtabstop=4: */
