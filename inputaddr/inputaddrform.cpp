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
#include "addrobjmodel.h"
#include "inputmodel.h"

#include <QCompleter>
#include <QIcon>
#include <QPlastiqueStyle>
#include <QRegExp>
#include <QValidator>


class InputFixup : public QValidator
{
public:
    InputFixup(QObject *parent = 0) : QValidator(parent) {}

    virtual State validate(QString &input, int &/*pos*/) const
        {
            if (input.isEmpty())
                return QValidator::Intermediate;

            static const QString tr_eng("`QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,.");
            static const QString tr_rus(QString::fromUtf8("ЁЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮ"));
            Q_ASSERT(tr_eng.size() == tr_rus.size());

            input.swap(input.toUpper()
                       .replace(QRegExp("\\s+"), " ")
                       .remove(QRegExp("^\\s+$?")));

            for (int k = 0; k < tr_eng.size(); ++k)
                input.swap(input.replace(tr_eng.at(k), tr_rus.at(k)));

            return QValidator::Acceptable;
        }
};

static void initInputEdit(QLineEdit *edit, QAbstractItemModel *model)
{
    QCompleter *completer = new QCompleter(edit);
    completer->setObjectName("completer");
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(model);
    completer->setWidget(edit);

    QObject::connect(edit, SIGNAL(textEdited(QString)), model, SLOT(search(QString)));
    QObject::connect(edit, SIGNAL(textEdited(QString)), completer, SLOT(setCompletionPrefix(QString)));
    QObject::connect(edit, SIGNAL(textEdited(QString)), completer, SLOT(complete()));
    QObject::connect(completer, SIGNAL(activated(QModelIndex)), model, SLOT(activated(QModelIndex)));
    QObject::connect(completer, SIGNAL(activated(QString)), edit, SLOT(setText(QString)));

    QValidator *validator = new InputFixup(edit);
    edit->setValidator(validator);
}

static QString threeDigitString(const int number, const QString &delim = " ")
{
    QString res = QString("%1").arg(number);
    const int size = res.size();

    if (size < 4) return res;

    for (int i = 3;  i < size; ++i)
        if (i % 3 == 0)
            res.insert(size - i, delim);

    return res;
}

InputAddrForm::InputAddrForm(QWidget *parent) :
    QDialog(parent),
    mModel(new AddrObjModel(this))
{
    QApplication::setStyle(new QPlastiqueStyle);
    setWindowIcon(QIcon(":/logo.png"));

    setupUi(this);
    connect(uiOkButton,     SIGNAL(clicked()), SLOT(accept()));
    connect(uiCancelButton, SIGNAL(clicked()), SLOT(reject()));
    uiOkButton->setEnabled(false);

    uiInfoLabel->setText(uiInfoLabel->text().arg(threeDigitString(mModel->addressObjectsCount())));

    uiRegionInput->setFocus();

    initInputEdit(uiRegionInput, mModel->regionModel());
    initInputEdit(uiAreaInput,   mModel->areaModel());
    initInputEdit(uiStreetInput, mModel->streetModel());

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));

    connect(mModel, SIGNAL(activatedStreetName(QString)), uiAddressLabel, SLOT(setText(QString)));
    connect(mModel, SIGNAL(activatedStreetName(QString)), SLOT(enableOkButton()));
}

void InputAddrForm::enableOkButton()
{
    uiOkButton->setEnabled(true);
}

void InputAddrForm::focusChanged(QWidget *old, QWidget */*now*/)
{
    if (old == uiRegionInput ||
        old == uiAreaInput ||
        old == uiStreetInput)
    {
        QLineEdit *edit = qobject_cast<QLineEdit *>(old);
        Q_ASSERT(edit);

        QCompleter *completer = edit->findChild<QCompleter *>("completer");
        Q_ASSERT(completer);

        InputModel *model = qobject_cast<InputModel *>(completer->model());
        Q_ASSERT(model);

        if (model->rowCount() > 0)
        {
            const QModelIndex idx = completer->currentIndex();
            model->activated(idx);
            edit->setText(model->data(idx).toString());
        }
    }
}

QString InputAddrForm::addressObject() const
{
    return mModel->addressObjectString();
}
