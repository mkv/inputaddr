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

#include "ui_progressdialog.h"

#include <QDialog>


class ProgressDialog : public QDialog, private Ui::ProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent = 0);

signals:
    void cancel();

public slots:
    void indexingProgress(int obj_count, int processed);

private:
    Q_DISABLE_COPY(ProgressDialog)

};
