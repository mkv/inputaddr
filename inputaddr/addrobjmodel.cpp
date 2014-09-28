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

#include "addrobjmodel.h"
#include "inputmodel.h"
#include "indexingthread.h"
#include "progressdialog.h"

#include <QAbstractItemModel>
#include <QDateTime>
#include <QFile>
#include <QSharedPointer>
#include <QTextStream>
#include <QUuid>

static const char DB_FILENAME[]    = "addrobj.tch";
static const char INDEX_FILENAME[] = "addrobj_index.tcb";


AddrObjModel::AddrObjModel(QObject *parent) :
    QObject(parent),
    hdbAddrObject(tchdbnew()),
    bdbIndex(tcbdbnew()),
    mRegionModel(new InputModel(ObjLevelRegion, bdbIndex, this)),
    mAreaModel(new InputModel(ObjLevelArea, bdbIndex, this)),
    mStreetModel(new InputModel(ObjLevelStreet, bdbIndex, this))
{
    if (!tchdbopen(hdbAddrObject, DB_FILENAME, HDBOREADER))
        qFatal("hdbAddrObject open error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));

    if (!QFile::exists(INDEX_FILENAME))
    {
        buildIndex();
        tcbdbclose(bdbIndex);
    }

    if (!tcbdbopen(bdbIndex, INDEX_FILENAME, BDBOREADER))
        buildIndex();

    if (!isValidIndex())
    {
        tcbdbclose(bdbIndex);
        buildIndex();
    }

    connect(mRegionModel, SIGNAL(activated(int,QByteArray)), SLOT(activated(int,QByteArray)));
    connect(mAreaModel,   SIGNAL(activated(int,QByteArray)), SLOT(activated(int,QByteArray)));
    connect(mStreetModel, SIGNAL(activated(int,QByteArray)), SLOT(activated(int,QByteArray)));
}

AddrObjModel::~AddrObjModel()
{
    if (!tchdbclose(hdbAddrObject))
        qWarning("hdbAddrObject close error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));
    tchdbdel(hdbAddrObject);

    if (!tcbdbclose(bdbIndex))
        qWarning("bdbIndex close error: %s", tcbdberrmsg(tcbdbecode(bdbIndex)));
    tcbdbdel(bdbIndex);
}

bool AddrObjModel::isValidIndex()
{
    QDateTime hdb_time; // address objects DB
    QDateTime bdb_time; // index

    char *value = NULL;
    int value_size = 0;

    value = (char *)tchdbget(hdbAddrObject, IMPORT_DATETIME_KEY, sizeof(IMPORT_DATETIME_KEY), &value_size);
    if (!value)
        return false;

    {
        QByteArray obj_data(value, value_size);
        QDataStream obj_stream(&obj_data, QIODevice::ReadOnly);
        obj_stream.setVersion(STREAM_VERSION);
        obj_stream >> hdb_time;
        free(value);
    }
    {
        BDBCUR *cur_index = tcbdbcurnew(bdbIndex);
        QSharedPointer<BDBCUR> cursor_cleanup(cur_index, tcbdbcurdel);

        if (!tcbdbcurjump(cur_index, IMPORT_DATETIME_KEY, sizeof(IMPORT_DATETIME_KEY)))
            return false;

        value = (char *)tcbdbcurval(cur_index, &value_size);
        if (!value)
            return false;

        QByteArray obj_data(value, value_size);
        QDataStream obj_stream(&obj_data, QIODevice::ReadOnly);
        obj_stream.setVersion(STREAM_VERSION);
        obj_stream >> bdb_time;
        free(value);
    }

    if (hdb_time == bdb_time)
        return true;

    return false;
}

void AddrObjModel::buildIndex()
{
    ProgressDialog dialog;
    dialog.indexingProgress(0, 0);

    IndexingThread index_worker(hdbAddrObject, bdbIndex, this);

    connect(&index_worker, SIGNAL(indexingProgress(int,int)),
            &dialog, SLOT(indexingProgress(int,int)));

    connect(&index_worker, SIGNAL(finished()), &dialog, SLOT(accept()));

    connect(&dialog, SIGNAL(cancel()), &index_worker, SLOT(cancel()));

    index_worker.start();
    dialog.exec();

    index_worker.cancel();
    if (!index_worker.wait(1000)) // 1 sec
        index_worker.terminate();
}

int AddrObjModel::addressObjectsCount() const
{
    return tchdbrnum(hdbAddrObject);
}

QString AddrObjModel::streetFullPath(const QByteArray &aoguid, const QString &delim) const
{
    const AddressObject obj = addressObjectByGuid(aoguid);
    return streetFullPath(QString("%1 %2").arg(obj.offname).arg(obj.shortname), obj.parentguid, delim);
}

QString AddrObjModel::streetFullPath(const QString &childname, const QByteArray &parentguid, const QString &delim) const
{
    int value_size;
    char *value = (char *)tchdbget(hdbAddrObject, parentguid.constData(), parentguid.size(), &value_size);
    if (value)
    {
        QByteArray obj_data(value, value_size);
        free(value);

        QDataStream obj_stream(&obj_data, QIODevice::ReadOnly);
        obj_stream.setVersion(STREAM_VERSION);

        AddressObject obj;
        obj_stream >> obj;

        const QString name = QString("%1 %2%3%4")
            .arg(obj.offname)
            .arg(obj.shortname)
            .arg(delim)
            .arg(childname);

        if (obj.parentguid.size() != 0)
            return streetFullPath(name, obj.parentguid, delim);
        else
            return name;
    }

    return childname;
}

QString AddrObjModel::addressObjectString() const
{
    const AddressObjectParents level_guid = parentsByGuid(mCurrentStreetGuid);

    const AddressObject region = addressObjectByGuid(level_guid[1]);
    const AddressObject area   = addressObjectByGuid(level_guid[3]);
    const AddressObject city   = addressObjectByGuid(level_guid[4]);
    const AddressObject town   = addressObjectByGuid(level_guid[6]);
    const AddressObject street = addressObjectByGuid(level_guid[7]);

    QString address;
    QTextStream out(&address);

    out << "addr_fullname:"     << streetFullPath(street.aoguid, ", ") << '\n'

        << "region_name:"       << region.offname << '\n'
        << "region_short:"      << region.shortname << '\n'

        << "area_name:"         << area.offname << '\n'
        << "area_short:"        << area.shortname << '\n'

        << "city_name:"         << city.offname << '\n'
        << "city_short:"        << city.shortname << '\n'

        << "town_name:"         << town.offname << '\n'
        << "town_short:"        << town.shortname << '\n'

        << "street_name:"       << street.offname << '\n'
        << "street_short:"      << street.shortname << '\n'

        << "street_aoguid:"
        << QUuid::fromRfc4122(street.aoguid).toString().remove("{").remove("}") << '\n'

        << "street_postalcode:" << street.postalcode << '\n';

    return address;
}

QAbstractItemModel* AddrObjModel::regionModel()
{
    return mRegionModel;
}

QAbstractItemModel* AddrObjModel::areaModel()
{
    return mAreaModel;
}

QAbstractItemModel* AddrObjModel::streetModel()
{
    return mStreetModel;
}

QByteArray AddrObjModel::indexKey(const int obj_level, const QString &name) const
{
    switch (obj_level)
    {
    case ObjLevelRegion:
        return QByteArray("1").append(name.toUtf8());
    case ObjLevelArea:
    case ObjLevelCity:
        return QByteArray("3").append(mCurrentRegionGuid).append(name.toUtf8());
    case ObjLevelTown:
    case ObjLevelStreet:
        return QByteArray("7").append(mCurrentRegionGuid).append(mCurrentAreaGuid).append(name.toUtf8());
        break;
    }

    return QByteArray();
}

QString AddrObjModel::objectName(const int obj_level, const QByteArray &aoguid) const
{
    const AddressObject obj = addressObjectByGuid(aoguid);
    if (obj.valid)
    {
        switch (obj_level)
        {
        case ObjLevelRegion:
            return QString("%1 %2").arg(obj.offname).arg(obj.shortname);
        case ObjLevelArea:
        case ObjLevelCity:
        case ObjLevelTown:
            return QString("%1 %2").arg(obj.offname).arg(obj.shortname);
        case ObjLevelStreet:
        {
            const AddressObject parent = addressObjectByGuid(obj.parentguid);

            return QString("%1 %2%3%4")
                .arg(obj.offname)
                .arg(obj.shortname)
                .arg(obj.parentguid != mCurrentAreaGuid ? QString(" %1 %2 %3")
                     .arg(QChar(0x2022)).arg(parent.offname).arg(parent.shortname) : "")
                .arg(obj.postalcode != 0 ? QString(" %1 %2")
                     .arg(QChar(0x2022)).arg(obj.postalcode) : "");
        }
        break;
        }
    }

    return QString();
}

void AddrObjModel::activated(const int obj_level, const QByteArray &guid)
{
    switch (obj_level)
    {
    case ObjLevelRegion:
        mCurrentRegionGuid = guid;
        break;
    case ObjLevelArea:
    case ObjLevelCity:
        mCurrentAreaGuid = guid;
        break;
    case ObjLevelTown:
        break;
    case ObjLevelStreet:
    {
        mCurrentStreetGuid = guid;
        emit activatedStreetName(streetFullPath(guid, ", "));
    }
    break;
    }
}

AddressObject AddrObjModel::addressObjectByGuid(const QByteArray &aoguid) const
{
    AddressObject obj;

    int value_size = 0;
    char *value = (char *)tchdbget(hdbAddrObject, aoguid.constData(), aoguid.size(), &value_size);
    if (value)
    {
        QByteArray obj_data(value, value_size);
        QDataStream obj_stream(&obj_data, QIODevice::ReadOnly);
        obj_stream.setVersion(STREAM_VERSION);
        obj_stream >> obj;
        free(value);
    }
    return obj;
}

AddressObjectParents AddrObjModel::parentsByGuid(const QByteArray &aoguid) const
{
    AddressObjectParents map;

    const AddressObject child = addressObjectByGuid(aoguid);
    map.insert(child.aolevel, child.aoguid);

    AddressObject parent = addressObjectByGuid(child.parentguid);
    while (parent.aoguid.size() != 0)
    {
        map.insert(parent.aolevel, parent.aoguid);
        parent = addressObjectByGuid(parent.parentguid);
    }

    return map;
}
