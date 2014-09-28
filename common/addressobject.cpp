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

#include "addressobject.h"

#include <QDataStream>
#include <QUuid>
#include <QXmlStreamAttributes>

AddressObject AddressObject::create(const QXmlStreamAttributes *attrs)
{
    Q_ASSERT(attrs);

    AddressObject obj;

    bool ok = false;
    obj.actstatus = attrs->value(QLatin1String("ACTSTATUS")).toString().toInt(&ok);
    if (!ok)
        return obj;

    obj.aolevel = attrs->value(QLatin1String("AOLEVEL")).toString().toShort(&ok);
    if (!ok)
        return obj;

    obj.aoguid     = QUuid(attrs->value(QLatin1String("AOGUID")).toString()).toRfc4122();
    obj.parentguid = QUuid(attrs->value(QLatin1String("PARENTGUID")).toString()).toRfc4122();

    obj.offname = attrs->value(QLatin1String("OFFNAME")).toString();
    if (obj.offname.isEmpty())
        return obj;

    obj.shortname = attrs->value(QLatin1String("SHORTNAME")).toString();
    if (obj.shortname.isEmpty())
        return obj;

    obj.postalcode = attrs->value(QLatin1String("POSTALCODE")).toString().toInt(&ok);

    obj.valid = true;

    return obj;
}

QDataStream &operator<<(QDataStream &stream, const AddressObject &obj)
{
    stream
        << obj.actstatus
        << obj.aolevel
        << obj.aoguid
        << obj.parentguid
        << obj.offname
        << obj.shortname
        << obj.postalcode;

    return stream;
}

QDataStream &operator>>(QDataStream &stream, AddressObject &obj)
{
    obj.valid = true;

    stream
        >> obj.actstatus
        >> obj.aolevel
        >> obj.aoguid
        >> obj.parentguid
        >> obj.offname
        >> obj.shortname
        >> obj.postalcode;

    return stream;
}
