#include "StandardItemModel.h"

StandardItemModel::StandardItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setupSignalsAndSlots();
}

StandardItemModel::~StandardItemModel() = default;

bool StandardItemModel::contains(const QString &szText, int nCol)
{
    if (nCol >= this->columnCount()) {
        return false;
    }

    const QList<QStandardItem *> myMatches = this->findItems(szText, Qt::MatchExactly, nCol);

    return (myMatches.isEmpty() == false); //if empty is false, contains is true
}

int StandardItemModel::indexOf(const QString &szText, int nCol, int nRowStart)
{
    if (nCol >= this->columnCount()) {
        return -1;
    }

    const QList<QStandardItem *> myMatches = this->findItems(szText, Qt::MatchExactly, nCol);

    if (myMatches.isEmpty() == true) {
        return -1;
    }

    for (const QStandardItem *myMatch : myMatches) {
        int nRow = myMatch->row();

        if (nRow >= nRowStart) {
            return nRow;
        }
    }

    return -1;
}

void StandardItemModel::insertRow(int row, const QList<QStandardItem *> &items)
{
    if (bAllowDuplicates == true) {
        QStandardItemModel::insertRow(row, items);

    } else {
        bool bExists = false;

        //for each row to add
        //get text from first column
        //find first row where that cell matches
        //check columns from that row until one cell to insert doesn't match with a cell on the model
        //get the next row where the first column matches and loop
        for (int nRow = 0; nRow < items.size(); nRow += this->columnCount()) {

            const QString szCell = items.at(nRow + 0)->text();

            int nMatchRow = -1;

            while ((nMatchRow = indexOf(szCell, 0, nMatchRow + 1)) != -1) {

                bool bRowMatches = true;

                for (int nCol = 0; nCol < this->columnCount(); ++nCol) {
                    if (items.at(nRow + nCol)->text() != this->item(nMatchRow, nCol)->text()) {
                        bRowMatches = false;
                        break;
                    }
                }

                if (bRowMatches == true) {
                    bExists = true;
                    break;
                }
            }

            if (bExists == true) {
                break;
            }
        }

        if (bExists == false) {
            QStandardItemModel::insertRow(row, items);
        }
    }


    sort();
}

void StandardItemModel::insertRow(int row, QStandardItem *item)
{
    if (bAllowDuplicates == true) {
        QStandardItemModel::insertRow(row, item);

    } else {
        if (contains(item->text(), 0) == false) {
            QStandardItemModel::insertRow(row, item);
        }
    }

    sort();
}

void StandardItemModel::appendRow(const QList<QStandardItem *> &items)
{
    insertRow(this->rowCount(), items);
}

void StandardItemModel::appendRow(QStandardItem *item)
{
    insertRow(this->rowCount(), item);
}

void StandardItemModel::setupSignalsAndSlots()
{
    connect(this,   &QStandardItemModel::itemChanged,
            this,   [ = ] { sort(); });
}

void StandardItemModel::sort(int column, Qt::SortOrder order)
{
    if (column != -1) {
        QStandardItemModel::sort(column, order);

    } else if (nSortColumn != -1) {
        QStandardItemModel::sort(nSortColumn);
    }
}
