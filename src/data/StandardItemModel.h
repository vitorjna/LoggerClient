#pragma once

#include <QDebug>
#include <QStandardItemModel>

class StandardItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit StandardItemModel(QObject *parent = nullptr);
    ~StandardItemModel() override;

    bool contains(const QString &szText, int nCol);

    int indexOf(const QString &szText, int nCol, int nRowStart = 0);

    void insertRow(int row, const QList<QStandardItem *> &items);
    void insertRow(int row, QStandardItem *item);

    void appendRow(const QList<QStandardItem *> &items);
    void appendRow(QStandardItem *item);

    inline bool getAllowDuplicates() const {
        return bAllowDuplicates;
    }
    inline void setAllowDuplicates(bool bAllowDuplicates) {
        this->bAllowDuplicates = bAllowDuplicates;
    }

    inline void setSortColumn(int nSortColumn) {
        this->nSortColumn = nSortColumn;
    }

protected:
    QColor myColorError;
    QBrush myBrushError;

private:
    void setupSignalsAndSlots();

    bool bAllowDuplicates = true;
    int nSortColumn = -1;


    // reimplemented
public:
    void sort(int column = -1, Qt::SortOrder order = Qt::AscendingOrder) override;

};


