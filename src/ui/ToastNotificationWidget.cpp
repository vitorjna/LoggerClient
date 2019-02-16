#include "ToastNotificationWidget.h"

const int   ToastNotificationWidget::nWidgetWidth;
const int   ToastNotificationWidget::nWidgetHeight;
QTimer      *ToastNotificationWidget::myTimer = new QTimer();

void ToastNotificationWidget::showMessage(QWidget *myContainer, const QString &szMessage, const NotificationType eNotifType, const int nTimeoutMs)
{
    ToastNotificationWidget *myWidget = new ToastNotificationWidget(eNotifType, myContainer);

    myWidget->labelMessage->setText(szMessage);
    myWidget->adjustSize();

    myWidget->move(myContainer->mapToGlobal(myContainer->rect().center()) - QPoint(nWidgetWidth / 2, nWidgetHeight / 2));
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
    QString szColor;

    switch (eNotifType) {
        case ToastNotificationWidget::SUCCESS:
            szColor = QColor(0, 192, 0, 192).name();
            break;

        case ToastNotificationWidget::ERROR:
        default:
            szColor = QColor(192, 0, 0, 192).name();
            break;

        case ToastNotificationWidget::INFO:
            szColor = QColor(192, 192, 0, 192).name();
            break;
    }

    this->setStyleSheet(QStringLiteral("background-color:%1;")
                        .arg(szColor)
                       );

    labelMessage->setStyleSheet(QStringLiteral("color:black;font-weight:bold;"));
}

void ToastNotificationWidget::setStyleSheet(const QString &szStyleSheet)
{
    QWidget::setStyleSheet(szStyleSheet);
}

ToastNotificationWidget::ToastNotificationWidget(const NotificationType eNotifType, QWidget *parent)
    : QWidget(parent)
    , labelMessage(nullptr)
{
    setupUi();
    setStyleSheet(eNotifType);
}

ToastNotificationWidget::~ToastNotificationWidget()
{
}

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
    resize(nWidgetWidth, nWidgetHeight);
}
