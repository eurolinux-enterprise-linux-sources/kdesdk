/*
    This file is part of the Okteta Kasten module, made within the KDE community.

    Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BYTEARRAYRAWFILERELOADJOB_H
#define BYTEARRAYRAWFILERELOADJOB_H

// Kasten core
#include <abstractfilesystemsyncfromremotejob.h>


namespace Kasten2
{

class ByteArrayRawFileSynchronizer;
class ByteArrayRawFileReloadThread;

class ByteArrayRawFileReloadJob : public AbstractFileSystemSyncFromRemoteJob
{
  Q_OBJECT

  public:
    ByteArrayRawFileReloadJob( ByteArrayRawFileSynchronizer *synchronizer );
    virtual ~ByteArrayRawFileReloadJob();

  protected: // AbstractFileSystemSyncFromRemoteJob API
    virtual void startReadFromFile();

  protected Q_SLOTS:
//     void onDataPulled();

  protected:
    ByteArrayRawFileReloadThread *mLoadThread;
};

}

#endif
