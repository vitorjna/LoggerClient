#include "StandardItemView.h"
#include "application/GlobalConstants.h"
#include "data/StandardItemModel.h"
#include "ui/ToastNotificationWidget.h"

StandardItemView::StandardItemView(QWidget *parent)
    : QTableView(parent)
    , myModel(new StandardItemModel(this))
{
}

StandardItemView::~StandardItemView() = default;

QString StandardItemView::toString(int nTop, int nBottom, int nLeft, int nRight)
{
    if (nBottom == -1) {
        nBottom = myModel->rowCount() - 1;
    }

    if (nRight == -1) {
        nRight = myModel->columnCount() - 1;
    }

    QStringList szaRows;
    szaRows.reserve(nBottom - nTop);

    QStringList szaRow;

    for (int nRow = nTop; nRow <= nBottom; ++nRow) {
        for (int nCol = nLeft; nCol <= nRight; ++nCol) {
            const QString szCellValue = myModel->index(nRow, nCol).data().toString();

            szaRow.append(szCellValue);
        }

        szaRows.append(szaRow.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_2));
        szaRow.clear();
    }

    QString szText = szaRows.join(GlobalConstants::SEPARATOR_SETTINGS_LIST);

    return szText;
}

void StandardItemView::setModel(StandardItemModel *model)
{
    this->myModel = model;

    QTableView::setModel(model);
}

void StandardItemView::keyPressEvent(QKeyEvent *myKeyEvent)
{
    if (myKeyEvent->key() == Qt::Key_Delete) {
        const QItemSelectionRange &mySelectionRange = selectionModel()->selection().first();
        myModel->removeRows(mySelectionRange.top(), mySelectionRange.height());

    } else if (myKeyEvent->matches(QKeySequence::Copy)) {
        if (selectedIndexes().isEmpty() == true) {
            return;
        }

        QString szText;

        const QItemSelection &myItemSelection = selectionModel()->selection();

        for (const QItemSelectionRange &mySelectionRange : myItemSelection) {
            szText.append(this->toString(mySelectionRange.top(), mySelectionRange.bottom(), mySelectionRange.left(), mySelectionRange.right()));
        }

        QApplication::clipboard()->setText(szText);

        ToastNotificationWidget::showMessage(this, tr("Data copied to clipboard"), ToastNotificationWidget::SUCCESS, 1500);

    } else if (myKeyEvent->matches(QKeySequence::Paste)) {
        const QString szClipboardData = QApplication::clipboard()->text();

        const QStringList szaRows = szClipboardData.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

        QList<QStandardItem *> myTableRow;

        for (int nRow = 0; nRow < szaRows.size(); ++nRow) {
            const QStringList szaRow = szaRows[nRow].split(GlobalConstants::SEPARATOR_SETTINGS_LIST_2);

            for (int nCol = 0; nCol < szaRow.size(); ++nCol) {
                myTableRow.append(new QStandardItem(szaRow[nCol]));
            }

            myModel->appendRow(myTableRow);
            myTableRow.clear();
        }

        ToastNotificationWidget::showMessage(this, tr("Clipboard parsed successfully"), ToastNotificationWidget::SUCCESS, 1000);

    } else {
        QTableView::keyPressEvent(myKeyEvent);
    }

}
