#include "LoggerTreeView.h"

LoggerTreeView::LoggerTreeView(QWidget *parent)
    : QTreeView(parent)
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

    QVector<int> naMaxSizes;
    naMaxSizes.reserve(nRight - nLeft);

    for (int nCol = nLeft; nCol <= nRight; ++nCol) {
        int nMaxSize = getColumnMaxWidth(nCol, nTop, nBottom);

        naMaxSizes.push_back(nMaxSize + (nMaxSize % 4) + 1); //round to the next multiple of 4
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
                szRowContents.append(szCellValue.leftJustified(naMaxSizes.at(nCol - nLeft), ' '));    //using spaces as separator. Using \t would change the message structure
            }
        }

        if (szRowContents.trimmed().isEmpty() == false) {
            szSelectionContents.append(szRowContents);
        }
    }

    QString szText = szSelectionContents.join('\n');
    return szText;
}

int LoggerTreeView::getColumnMaxWidth(const int nCol, int nRowTop, int nRowBottom)
{
    if (nRowBottom == -1) {
        nRowBottom = this->model()->rowCount() - 1;
    }

    int nMaxSize = -1;

    for (int nRow = nRowTop; nRow <= nRowBottom; ++nRow) {
        int nCellSize = model()->index(nRow, nCol).data().toString().size();

        if (nCellSize > nMaxSize) {
            nMaxSize = nCellSize;
        }
    }

    return nMaxSize;
}

void LoggerTreeView::keyPressEvent(QKeyEvent *myKeyEvent)
{
    if (myKeyEvent->key() == Qt::Key_Delete) {
        foreach (QModelIndex index, selectedIndexes()) {
            model()->removeRow(index.row());
        }

    } else if (myKeyEvent->matches(QKeySequence::Copy)) {
        if (selectedIndexes().isEmpty() == true) {
            return;
        }

        QItemSelectionRange selectionRange = selectionModel()->selection().first();
        QString szText = this->toString(selectionRange.top(), selectionRange.bottom(), selectionRange.left(), selectionRange.right());
        QApplication::clipboard()->setText(szText);

    } else {
        QTreeView::keyPressEvent(myKeyEvent);
    }

}
