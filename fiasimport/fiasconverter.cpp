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

#include "fiasconverter.h"
#include "addressobject.h"

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QXmlStreamReader>

FiasConverter::FiasConverter() :
    hdbAddrObject(tchdbnew())
{
    if (!tchdbopen(hdbAddrObject, "addrobj.tch", HDBOWRITER | HDBOCREAT | HDBOTRUNC))
        qFatal("hdb open error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));
}

FiasConverter::~FiasConverter()
{
    if (!tchdbclose(hdbAddrObject))
        qWarning("hdbAddrObject close error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));

    tchdbdel(hdbAddrObject);
}

void FiasConverter::process(const QString &addrobj_file)
{
    QFile addrobj_xml_file(addrobj_file);

    if (!addrobj_xml_file.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("Failed to open XML file");

    QXmlStreamReader mXml;
    mXml.setDevice(&addrobj_xml_file);

    if (mXml.readNextStartElement() && mXml.name() != QLatin1String("AddressObjects"))
        qFatal("The file is not an FIAS file");

    if (mXml.readNextStartElement())
    {
        while (!mXml.atEnd())
        {
            if (mXml.isStartElement() && mXml.name() == QLatin1String("Object"))
            {
                const QXmlStreamAttributes attrs = mXml.attributes();
                AddressObject obj = AddressObject::create(&attrs);

                if (!obj.valid)
                {
                    mXml.readNext();
                    continue;
                }

                QByteArray  obj_data;
                QDataStream obj_stream(&obj_data, QIODevice::WriteOnly);
                obj_stream.setVersion(STREAM_VERSION);

                obj_stream << obj;

                if (!tchdbput(hdbAddrObject, obj.aoguid.constData(), obj.aoguid.size(), obj_data.constData(), obj_data.size()))
                    qFatal("put error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));
            }
            mXml.readNext();
        }
    }

    if (mXml.hasError())
    {
        qFatal(QObject::tr("%1. Line %2, column %3")
               .arg(mXml.errorString())
               .arg(mXml.lineNumber())
               .arg(mXml.columnNumber()).toLatin1().constData());
    }

    saveImportTime();
}

void FiasConverter::saveImportTime()
{
    QByteArray  data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(STREAM_VERSION);

    stream << QDateTime::currentDateTime();

    if (!tchdbput(hdbAddrObject, IMPORT_DATETIME_KEY, sizeof(IMPORT_DATETIME_KEY),
                  data.constData(), data.size()))
        qFatal("put error: %s", tchdberrmsg(tchdbecode(hdbAddrObject)));
}
