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

#include "indexingthread.h"
#include "addressobject.h"
#include "addrobjmodel.h"

#include <QByteArray>
#include <QDataStream>


void IndexingThread::insertIndex(const QByteArray &key, const QByteArray &value)
{
    if (!tcbdbputdup(bdbIndex, key.constData(), key.size(), value.constData(), value.size()))
        qFatal("put error: %s", tcbdberrmsg(tcbdbecode(bdbIndex)));
}

void IndexingThread::run()
{
    if (!tcbdbopen(bdbIndex, "addrobj_index.tcb", BDBOREADER | BDBOWRITER | BDBOCREAT | HDBOTRUNC))
        qFatal("bdb open error: %s", tcbdberrmsg(tcbdbecode(bdbIndex)));

    const int obj_count = tchdbrnum(hdbAddrObject);
    int processed = 0;

    tchdbiterinit(hdbAddrObject);

    void *key;
    int key_size;
    while ((key = tchdbiternext(hdbAddrObject, &key_size)) != NULL && !mStop)
    {
        int value_size;
        char *value = (char *)tchdbget(hdbAddrObject, key, key_size, &value_size);

        if (value)
        {
            if (::strcmp((char *)key, IMPORT_DATETIME_KEY) == 0)
            {
                if (!tcbdbputdup(bdbIndex, key, key_size, value, value_size))
                    qFatal("put error: %s", tcbdberrmsg(tcbdbecode(bdbIndex)));

                free(value);
                free(key);
                continue;
            }

            QByteArray obj_data(value, value_size);
            QDataStream obj_stream(&obj_data, QIODevice::ReadOnly);
            obj_stream.setVersion(STREAM_VERSION);

            AddressObject obj;
            obj_stream >> obj;

            const AddressObjectParents parents = mParent->parentsByGuid(obj.aoguid);
            const QByteArray name = obj.offname.toUpper().toUtf8();

            switch (obj.aolevel)
            {
            case 1: // 1 – уровень региона (region)
                insertIndex(QByteArray("1")
                            .append(name),
                            obj.aoguid);
                break;
            case 3: // 3 – уровень района (area)
            case 4: // 4 – уровень города (city)
                insertIndex(QByteArray("3")
                            .append(parents[1])
                            .append(name),
                            obj.aoguid);
                break;
            case 6: // 6 – уровень населенного пункта (town)
                break;
            case 7: // 7 – уровень улицы (street)
                insertIndex(QByteArray("7")
                            .append(parents[1])
                            .append(parents[3])
                            .append(name),
                            obj.aoguid);

                insertIndex(QByteArray("7")
                            .append(parents[1])
                            .append(parents[4])
                            .append(name),
                            obj.aoguid);

                const QByteArray name_prefix("ИМ ");
                if (name.startsWith(name_prefix))
                {
                    const QByteArray simply_name = name.mid(name_prefix.size());

                    insertIndex(QByteArray("7")
                                .append(parents[1])
                                .append(parents[3])
                                .append(simply_name),
                                obj.aoguid);

                    insertIndex(QByteArray("7")
                                .append(parents[1])
                                .append(parents[4])
                                .append(simply_name),
                                obj.aoguid);
                }
                break;
            }
            free(value);
        }
        free(key);

        ++processed;

        if (processed % 1000 == 0)
            emit indexingProgress(obj_count, processed);
    }
}
