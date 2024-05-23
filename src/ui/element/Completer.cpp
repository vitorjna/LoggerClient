#include "Completer.h"

bool Completer::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return
            || keyEvent->key() == Qt::Key_Enter) {

            QAbstractItemView *view = qobject_cast<QAbstractItemView *>(watched);
            QString szCurrentSuggestion = view->currentIndex().data(Qt::DisplayRole).toString();

            if (szCurrentSuggestion.isEmpty() == true) { //if not using model but instead writing directly in the line edit
                szCurrentSuggestion = currentCompletion();
            }

            emit activated(szCurrentSuggestion);

            view->hide();

            return true;
        }
    }

    return QCompleter::eventFilter(watched, event);
}
