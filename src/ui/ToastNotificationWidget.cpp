#include "ToastNotificationWidget.h"

const int   ToastNotificationWidget::nWidgetWidth;
const int   ToastNotificationWidget::nWidgetHeight;
QTimer      *ToastNotificationWidget::myTimer = new QTimer();

void ToastNotificationWidget::showMessage(QWidget *myContainer, const QString &szMessage, const NotificationType eNotifType, const int nTimeoutMs)
{
    ToastNotificationWidget *myWidget = new ToastNotificationWidget(eNotifType, myContainer);

    myWidget->labelMessage->setText(szMessage);
    myWidget->adjustSize();

    if (myContainer == nullptr) { //TODO toasts don't work if the container is not visible yet. They're also not mapping correctly to the desktop as a fallback
        myWidget->move(myContainer->mapToGlobal(QApplication::primaryScreen()->geometry().center()));

    } else {
        myWidget->move(myContainer->mapToGlobal(myContainer->rect().center()) - myWidget->rect().center());
    }

    myWidget->show();

    if (nTimeoutMs != -1) {
        //destroy any other Toast
        if (myTimer->isActive() == true) {
            QTimerEvent myEvent{myTimer->timerId()};
            QCoreApplication::sendEvent(myTimer, &myEvent);
            myTimer->stop();
        }

        QObject::connect(myTimer, SIGNAL(timeout()), myWidget, SLOT(close()));

        myTimer->start(nTimeoutMs);
    }
}

void ToastNotificationWidget::setStyleSheet(const NotificationType eNotifType)
{
    switch (eNotifType) {
        case ToastNotificationWidget::SUCCESS: {
            static const QString szColor = QColor(0, 192, 0, 192).name();
            this->setStyleSheet(QStringLiteral("background-color:%1;").arg(szColor));
            break;
        }

        case ToastNotificationWidget::ERROR:
        case ToastNotificationWidget::COUNT_NOTIF_TYPES: {
            static const QString szColor = QColor(192, 0, 0, 192).name();
            this->setStyleSheet(QStringLiteral("background-color:%1;").arg(szColor));
            break;
        }

        case ToastNotificationWidget::INFO: {
            static const QString szColor = QColor(192, 192, 0, 192).name();
            this->setStyleSheet(QStringLiteral("background-color:%1;").arg(szColor));
            break;
        }
    }

    labelMessage->setStyleSheet(QStringLiteral("color:black;font-weight:bold;"));
}

void ToastNotificationWidget::setStyleSheet(const QString &szStyleSheet)
{
    QWidget::setStyleSheet(szStyleSheet);
}

ToastNotificationWidget::ToastNotificationWidget(const NotificationType eNotifType, QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setStyleSheet(eNotifType);
}

ToastNotificationWidget::~ToastNotificationWidget() = default;

void ToastNotificationWidget::setupUi()
{
    this->hide();

    QVBoxLayout *myMainLayout = new QVBoxLayout(this);
    myMainLayout->setSpacing(0);
    myMainLayout->setContentsMargins(5, 5, 5, 5);

    labelMessage = new QLabel(this);
    labelMessage->setWordWrap(true);
    labelMessage->setAlignment(Qt::AlignCenter);

    myMainLayout->addWidget(labelMessage, 0, Qt::AlignHCenter);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setMinimumSize(nWidgetWidth, nWidgetHeight);
}
