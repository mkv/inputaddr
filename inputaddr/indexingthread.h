/*
 * Copyright (C) 2014 Konstantin Molchanov <molchanov.kv@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 *
 * For more details see the file COPYING.
 */

#pragma once

#include <QThread>

#include <tcbdb.h>

class AddrObjModel;

class IndexingThread : public QThread
{
    Q_OBJECT

public:
    IndexingThread(TCHDB *hdb, TCBDB *bdb, AddrObjModel *parent) :
        QThread(),
        mParent(parent),
        mStop(false),
        hdbAddrObject(hdb),
        bdbIndex(bdb)
        {}

public slots:
    void cancel() { mStop = true; }

signals:
    void indexingProgress(int obj_count, int processed);

protected:
    void run();

private:
    Q_DISABLE_COPY(IndexingThread)

    void insertIndex(const QByteArray &key, const QByteArray &value);

    AddrObjModel *mParent;

    bool mStop;

    TCHDB *hdbAddrObject;
    TCBDB *bdbIndex;

};
