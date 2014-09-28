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

#include "inputmodel.h"


InputModel::InputModel(const int obj_level, TCBDB *bdb, AddrObjModel *parent) :
    QAbstractItemModel(parent),
    mParentModel(parent),
    mAddrObjLevel(obj_level),
    curIndex(tcbdbcurnew(bdb)),
    mRowCount(0),
    mCurrentRow(-1)
{
}

bool InputModel::setCursor(const int index) const
{
    if (mCurrentRow == -1)
        return false;

    if (mCurrentRow == index)
        return true;

    if (mCurrentRow > index)
    {
        for (int i = 0; i < (mCurrentRow - index); ++i)
            if (!tcbdbcurprev(curIndex))
            {
                for (int k = 0; k < i; ++k)
                    tcbdbcurnext(curIndex);
                return false;
            }
    }
    else
    {
        for (int i = 0; i < (index - mCurrentRow); ++i)
            if (!tcbdbcurnext(curIndex))
            {
                for (int k = 0; k < i; ++k)
                    tcbdbcurprev(curIndex);
                return false;
            }
    }

    mCurrentRow = index;
    return true;
}

void InputModel::search(const QString &name)
{
    int count = 0;
    int key_size = 0;
    int offset = 0;

    const QByteArray &search_index = mParentModel->indexKey(mAddrObjLevel, name);

    if (name.size() != 0 && tcbdbcurjump(curIndex, search_index.constData(), search_index.size()))
    {
        do
        {
            if (void *key = tcbdbcurkey(curIndex, &key_size))
            {
                const bool keys_equal = ::memcmp(key, search_index.constData(), qMin(key_size, search_index.size())) == 0;
                free(key);

                if (keys_equal)
                    ++count;
                else
                    if (offset > 0) break;
            }
        }
        while (tcbdbcurnext(curIndex) && ++offset);

        for (int i = 0; i < offset; ++i)
            tcbdbcurprev(curIndex);
    }

    mRowCount = count;
    mCurrentRow = 0;
    emit layoutChanged();
}

QVariant InputModel::data(const QModelIndex &index, int role) const
{
    setCursor(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole)
    {
        int aoguid_size = 0;
        char *aoguid = (char*)tcbdbcurval(curIndex, &aoguid_size);
        if (aoguid)
        {
            const QByteArray guid(aoguid, aoguid_size);
            free(aoguid);

            if (role == Qt::UserRole)
                return guid;
            else
                return mParentModel->objectName(mAddrObjLevel, guid);
        }
    }

    return QVariant();
}

void InputModel::activated(const QModelIndex &index)
{
    emit activated(mAddrObjLevel, data(index, Qt::UserRole).toByteArray());
}

int InputModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int InputModel::rowCount(const QModelIndex &) const
{
    return mRowCount;
}

QVariant InputModel::headerData(int /* section */, Qt::Orientation /* orientation */, int /*role*/) const
{
    return QVariant();
}

QModelIndex InputModel::index(int row, int column, const QModelIndex &/*parent*/) const
{
    return createIndex(row, column);
}

QModelIndex InputModel::parent(const QModelIndex &/*parent*/) const
{
    return QModelIndex();
}
