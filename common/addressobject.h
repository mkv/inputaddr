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

#include <QByteArray>
#include <QDataStream>
#include <QMap>
#include <QString>

const QDataStream::Version STREAM_VERSION = QDataStream::Qt_4_8;
const char IMPORT_DATETIME_KEY[] = "__ADDRESS_OBJECTS_IMPORT_DATETIME_KEY";

class QXmlStreamAttributes;

class AddressObject
{
public:
    AddressObject() : valid(false), actstatus(false), aolevel(0), postalcode(0) {}

    static AddressObject create(const QXmlStreamAttributes *attrs);

    bool valid;

    // Статус актуальности адресного объекта ФИАС (0 – Не актуальный, 1 - Актуальный)
    bool actstatus;

    // Уровень адресного объекта
    short aolevel;

    // Глобальный уникальный идентификатор адресного объекта
    QByteArray aoguid;

    // Идентификатор объекта родительского объекта
    QByteArray parentguid;

    // Официальное наименование
    QString offname;

    // Краткое наименование типа объекта
    QString shortname;

    // Почтовый индекс
    int postalcode;
};

QDataStream &operator<<(QDataStream &stream, const AddressObject &obj);
QDataStream &operator>>(QDataStream &stream, AddressObject &obj);

typedef QMap<int /*aolevel*/, QByteArray /*aoguid*/> AddressObjectParents;
