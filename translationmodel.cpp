#include "translationmodel.h"

TranslationModel::TranslationModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant TranslationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    if (Qt::DisplayRole == role) {
        if (Qt::Horizontal == orientation)
            switch (section) {
            case 0: return QString("выражение"); break;
            case 1: return QString("перевод"); break;
            case 2: return QString("направление"); break;
            default: break;
            }
        //else if (Qt::Vertical == orientation)
        //   return QVariant();
    }
    return QVariant();
}

/*bool TranslationModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}*/


int TranslationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

int TranslationModel::columnCount(const QModelIndex &parent) const
{
    //if (parent.isValid())
    //    return 0;

    // FIXME: Implement me!
    return 3;
}

QVariant TranslationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    //const int row = index.row();
    //const int column = index.column();
    const QString firstExpr = items[index.row()]->firstExpr();
    const QString secondExpr = items[index.row()]->secondExpr();
    const bool invTranslation = items[index.row()]->isInvert();
    if (0 == index.column())
        return invTranslation ? firstExpr : secondExpr;
    else if (1 == index.column())
        return invTranslation ? secondExpr : firstExpr;
    else if (2 == index.column())
        return invTranslation ? QVariant("<--") : QVariant("-->");
    return QVariant();
}

bool TranslationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags TranslationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable; // FIXME: Implement me!
}

bool TranslationModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
}

/*bool TranslationModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
}*/

bool TranslationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
}

/*bool TranslationModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
}*/
