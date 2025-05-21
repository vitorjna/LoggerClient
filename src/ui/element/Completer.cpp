#include "Completer.h"

bool Completer::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {

            QAbstractItemView *view = qobject_cast<QAbstractItemView *>(watched);

            if (view != nullptr) {
                QString szCurrentSuggestion = view->currentIndex().data(Qt::DisplayRole).toString();

                if (szCurrentSuggestion.isEmpty() == true) { //if not using model but instead writing directly in the line edit
                    szCurrentSuggestion = currentCompletion();
                }

                Q_EMIT activated(szCurrentSuggestion);

                view->hide();

            } else {
                Q_EMIT activated("");
            }

            return true;
        }
    }

    return QCompleter::eventFilter(watched, event);
}
