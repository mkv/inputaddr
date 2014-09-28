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

#include "progressdialog.h"

#include <QApplication>
#include <QPlastiqueStyle>
#include <QIcon>


ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent)
{
    QApplication::setStyle(new QPlastiqueStyle);
    setWindowIcon(QIcon(":/logo.png"));

    setupUi(this);
    uiProgressBar->setValue(0);

    connect(uiCancelButton, SIGNAL(clicked(bool)), this, SIGNAL(cancel()));
}

void ProgressDialog::indexingProgress(int obj_count, int processed)
{
    uiProgressBar->setRange(0, obj_count);
    uiProgressBar->setValue(processed);
    uiRecordsCountLabel->setText(QString("%1").arg(obj_count));
    uiLabelProcessedRec->setText(QString("%1").arg(processed));
}
