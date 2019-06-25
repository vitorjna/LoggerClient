#include "LoggerTreeView.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"

LoggerTreeView::LoggerTreeView(QWidget *parent)
    : QTableView(parent)
{
}

LoggerTreeView::~LoggerTreeView()
{
}

QString LoggerTreeView::toString(int nTop, int nBottom, int nLeft, int nRight)
{
    if (nBottom == -1) {
        nBottom = this->model()->rowCount() - 1;
    }

    if (nRight == -1) {
        nRight = this->model()->columnCount() - 1;
    }

    //remove the line index column
    if (nLeft == 0) {
        ++nLeft; //TODO use a check by name/visibility
    }


    bool bFormatExportedLogs = AppSettings::getValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, false).toBool();

    QVector<int> naMaxSizes;

    if (bFormatExportedLogs == true) {
        naMaxSizes.reserve(nRight - nLeft);

        for (int nCol = nLeft; nCol <= nRight; ++nCol) {
            int nMaxCharCount = getColumnMaxCharCount(nCol, nTop, nBottom);

            naMaxSizes.push_back(nMaxCharCount + (nMaxCharCount % 4) + 1); //round to the next multiple of 4
        }
    }

    QStringList szSelectionContents;
    szSelectionContents.reserve(100 * (nBottom - nTop));

    for (int nRow = nTop; nRow <= nBottom; ++nRow) {
        QString szRowContents;
        szRowContents.reserve(100);

        for (int nCol = nLeft; nCol <= nRight; ++nCol) {
            const QString szCellValue = model()->index(nRow, nCol).data().toString();

            if (nCol == nRight) {
                szRowContents.append(szCellValue); //for the last column, no need to indent

            } else {
                if (bFormatExportedLogs == true) {
                    szRowContents.append(szCellValue.leftJustified(naMaxSizes.at(nCol - nLeft), GlobalConstants::SEPARATOR_EXPORTED_TEXT_COLUMN)); //using spaces as separator. Using \t would change the message structure

                } else {
                    szRowContents.append(szCellValue + GlobalConstants::SEPARATOR_EXPORTED_TEXT_COLUMN);
                }
            }
        }

        if (szRowContents.trimmed().isEmpty() == false) {
            szSelectionContents.append(szRowContents);
        }
    }

    QString szText = szSelectionContents.join(GlobalConstants::SEPARATOR_EXPORTED_TEXT_LINE);
    szText.append(GlobalConstants::SEPARATOR_EXPORTED_TEXT_LINE);

    return szText;
}

int LoggerTreeView::getColumnMaxCharCount(const int nCol, const int nRowTop, int nRowBottom, bool bFixOutliers)
{
    if (nRowBottom == -1) {
        nRowBottom = this->model()->rowCount() - 1;
    }

    int nMaxSize = -1;

    if (bFixOutliers == true) {
        QVector<int> naMaxSizes;
        naMaxSizes.reserve(nRowBottom - nRowTop);

        for (int nRow = nRowTop; nRow <= nRowBottom; ++nRow) {
            naMaxSizes.append(model()->index(nRow, nCol).data().toString().size());
        }

        std::sort(naMaxSizes.begin(), naMaxSizes.end());

        nMaxSize = naMaxSizes.at(qRound(naMaxSizes.size() * 0.95));

    } else {
        for (int nRow = nRowTop; nRow <= nRowBottom; ++nRow) {
            int nCellSize = model()->index(nRow, nCol).data().toString().size();

            if (nCellSize > nMaxSize) {
                nMaxSize = nCellSize;
            }
        }
    }

    return nMaxSize;
}

void LoggerTreeView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    qDebug() << index.row() << index.column();

    if (hint == QAbstractItemView::EnsureVisible
        && index.column() == index.model()->columnCount() - 1) { //last column
        return;
    }

    QTableView::scrollTo(index, hint);
}

void LoggerTreeView::keyPressEvent(QKeyEvent *myKeyEvent)
{
    if (myKeyEvent->key() == Qt::Key_Delete) {
        //TODO can't delete multiple selections, as the indexes change and a selection of index 5 and 10 would delete 5 and then 11, which is now 10
//        const QItemSelection myItemSelection = selectionModel()->selection();
//        for (const QItemSelectionRange &selectionRange : myItemSelection) {
        const QItemSelectionRange &mySelectionRange = selectionModel()->selection().first();
        model()->removeRows(mySelectionRange.top(), mySelectionRange.height());
//        }

    } else if (myKeyEvent->matches(QKeySequence::Copy)) {
        QModelIndexList myModelIndexList = selectedIndexes();

        if (myModelIndexList.isEmpty() == true) {
            return;
        }

        QString szText;

        const QItemSelection &myItemSelection = selectionModel()->selection();

        //TODO export follows selection order, not index order
        for (const QItemSelectionRange &mySelectionRange : myItemSelection) {
            szText.append(this->toString(mySelectionRange.top(), mySelectionRange.bottom(), mySelectionRange.left(), mySelectionRange.right()));
        }

        QApplication::clipboard()->setText(szText);

    } else {
        QTableView::keyPressEvent(myKeyEvent);
    }

}
