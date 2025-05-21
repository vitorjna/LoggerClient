#include "LineEditWidget.h"

LineEditWidget::LineEditWidget(QWidget *parent)
    : QLineEdit(parent)
{
}

LineEditWidget::~LineEditWidget() = default;

void LineEditWidget::focusInEvent(QFocusEvent *event)
{
    Q_EMIT focusReceived();

    QLineEdit::focusInEvent(event);
}

