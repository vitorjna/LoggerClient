#pragma once

#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>

/**
 * This class is used to reimplement eventFilter and catch the returnPressed event
 * before it is propagated to the parent QLineEdit
 * Without this, QLineEdit would emit returnPressed before the QCompleter suggestion was placed in the right place,
 * making the QLineEdit returnPressed use the previously selected values
 */
class Completer : public QCompleter
{
    Q_OBJECT

public:
    using QCompleter::QCompleter; //inherit constructors

    //reimplemented
    bool eventFilter(QObject *watched, QEvent *event) override;
};
