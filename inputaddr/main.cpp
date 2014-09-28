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

#include "inputaddrform.h"

#include <QApplication>
#include <QClipboard>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator app_tr;
    app_tr.load(":/app_ru.qm");
    app.installTranslator(&app_tr);

    QTranslator qt_tr;
    qt_tr.load(":/qt_ru.qm");

    if (qt_tr.isEmpty())
        qt_tr.load(QLibraryInfo::location(QLibraryInfo::TranslationsPath)+"/qt_ru.qm");
    else
        app.installTranslator(&qt_tr);

    InputAddrForm inputaddrform;
    inputaddrform.show();

    if (inputaddrform.exec() == QDialog::Accepted)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(inputaddrform.addressObject());

        QTextStream out(stdout);
        out << clipboard->text();
        out.flush();

        return 0;
    }

    return 1;
}
