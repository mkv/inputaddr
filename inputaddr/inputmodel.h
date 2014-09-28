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

#include "addrobjmodel.h"
#include "addressobject.h"

#include <QAbstractItemModel>
#include <QThread>

#include <tcbdb.h>

class InputModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    InputModel(const int obj_level, TCBDB *bdb, AddrObjModel *parent);

    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

signals:
    void activated(const int obj_level, const QByteArray &guid);

public slots:
    void search(const QString &name);
    void activated(const QModelIndex &index);

private:
    Q_DISABLE_COPY(InputModel)

    bool setCursor(const int index) const;

    AddrObjModel *mParentModel;
    int mAddrObjLevel;

    BDBCUR *curIndex;

    int mRowCount;

    mutable int mCurrentRow;
    mutable AddressObject mCurrentAddrObj;

};
