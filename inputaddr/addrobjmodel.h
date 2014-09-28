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

#include "addressobject.h"

#include <QByteArray>
#include <QModelIndex>

#include <tcbdb.h>

class InputModel;
class QAbstractItemModel;

class AddrObjModel : public QObject
{
    Q_OBJECT

public:
    AddrObjModel(QObject *parent = 0);
    ~AddrObjModel();

    int addressObjectsCount() const;

    QString addressObjectString() const;

    QAbstractItemModel* regionModel();
    QAbstractItemModel* areaModel();
    QAbstractItemModel* streetModel();

    QByteArray indexKey(const int obj_level, const QString &name) const;
    QString objectName(const int obj_level, const QByteArray &aoguid) const;

    enum AddrObjLevel
    {
        ObjLevelRegion = 1, // уровень региона (region)
        ObjLevelArea   = 3, // уровень района (area)
        ObjLevelCity   = 4, // уровень города (city)
        ObjLevelTown   = 6, // уровень населенного пункта (town)
        ObjLevelStreet = 7  // уровень улицы (street)
    };

    AddressObject addressObjectByGuid(const QByteArray &guid) const;
    AddressObjectParents parentsByGuid(const QByteArray &guid) const;

signals:
    void activatedStreetName(const QString &name);

public slots:
    void activated(const int obj_level, const QByteArray &guid);

private:
    Q_DISABLE_COPY(AddrObjModel)

    QString streetFullPath(const QByteArray &aoguid, const QString &delim) const;
    QString streetFullPath(const QString &childname, const QByteArray &parentguid, const QString &delim) const;

    bool isValidIndex();
    void buildIndex();

    TCHDB *hdbAddrObject;
    TCBDB *bdbIndex;

    InputModel *mRegionModel;
    InputModel *mAreaModel;
    InputModel *mStreetModel;

    QByteArray mCurrentRegionGuid;
    QByteArray mCurrentAreaGuid;
    QByteArray mCurrentStreetGuid;

};
