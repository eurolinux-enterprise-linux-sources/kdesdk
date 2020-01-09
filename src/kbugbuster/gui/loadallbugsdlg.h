/***************************************************************************
       loadallbugsdlg.h  -  progress dialog while loading all bugs for a package
                             -------------------
    copyright            : (C) 2002 by David Faure
    email                : david@mandrakesoft.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2.                               *
 *                                                                         *
 ***************************************************************************/
#ifndef loadallbugsdlg_h
#define loadallbugsdlg_h

#include <kdialog.h>
#include "bug.h"
class Package;
class BugDetails;
class KProgressDialog;

class LoadAllBugsDlg : public KDialog
{
    Q_OBJECT
public:
    LoadAllBugsDlg( const Package& pkg, const QString &component );

protected slots:
    void slotBugDetailsAvailable( const Bug &bug, const BugDetails &bd );
    void slotBugDetailsLoadingError();
    void slotStopped();
    void loadNextBug();
private:
    Bug::List m_bugs;
    unsigned int m_processed;
    unsigned int m_count;
    KProgressDialog* m_bugLoadingProgress;
};

#endif
