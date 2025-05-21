#pragma once

#include <QDebug>
#include <QLineEdit>

class LineEditWidget : public QLineEdit
{
    Q_OBJECT

public:
    explicit LineEditWidget(QWidget *parent = nullptr);
    ~LineEditWidget() override;

Q_SIGNALS:
    void focusReceived();

    //reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};
