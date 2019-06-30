#include "LineEditWidget.h"

LineEditWidget::LineEditWidget(QWidget *parent)
    : QLineEdit(parent)
{
}

LineEditWidget::~LineEditWidget() = default;

void LineEditWidget::focusInEvent(QFocusEvent *event)
{
    emit focusReceived();

    QLineEdit::focusInEvent(event);
}

