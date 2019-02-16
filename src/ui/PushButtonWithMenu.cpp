#include "PushButtonWithMenu.h"

PushButtonWithMenu::PushButtonWithMenu(QWidget *parent)
    : QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);
//    setToolButtonStyle(Qt::ToolButtonTextOnly);

    connect(this,   SIGNAL(triggered(QAction *)),
            this,   SLOT(setDefaultAction(QAction *)));
}

PushButtonWithMenu::~PushButtonWithMenu()
{
}

void PushButtonWithMenu::addAction(QAction *myAction)
{
    if (myAction == nullptr) {
        return;
    }

    QToolButton::addAction(myAction);

    if (this->actions().size() == 1) {
        this->setDefaultAction(myAction);
    }

}

void PushButtonWithMenu::addActions(QList<QAction *> myActionList)
{
    foreach (QAction *myAction, myActionList) {
        this->addAction(myAction);
    }
}
