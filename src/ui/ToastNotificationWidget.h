#pragma once

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QWidget>

class ToastNotificationWidget : public QWidget
{
    Q_OBJECT

public:
    enum NotificationType {
        SUCCESS,
        ERROR,
        INFO,
        COUNT_NOTIF_TYPES
    };

    static void showMessage(QWidget *myContainer, const QString &szMessage, const NotificationType eNotifType, const int nTimeoutMs);

    void setStyleSheet(const NotificationType eNotifType);
    void setStyleSheet(const QString &szStyleSheet);

private:
    static const int nWidgetWidth   = 150;
    static const int nWidgetHeight  = 64;

    static QTimer *myTimer;

    explicit ToastNotificationWidget(const NotificationType eNotifType, QWidget *parent = nullptr);
    ~ToastNotificationWidget() override;

    void setupUi();

    QLabel          *labelMessage{};

};
