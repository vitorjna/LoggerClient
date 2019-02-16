#pragma once

#include <QAction>
#include <QDebug>
#include <QToolButton>

class PushButtonWithMenu : public QToolButton
{
    Q_OBJECT

public:
    explicit PushButtonWithMenu(QWidget *parent = nullptr);
    ~PushButtonWithMenu() override;

    void addAction(QAction *myAction);

    void addActions(QList<QAction *> myActionList);

};
