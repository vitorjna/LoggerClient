#pragma once

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>

#include "data/NetworkAddressesModel.h"

class StandardItemView;

class NetworkAddressesManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkAddressesManagerWidget(QWidget *parent = nullptr);
    ~NetworkAddressesManagerWidget() override;

    QStringList getRow(const QString &szFind, const NetworkAddressesEnum::Columns eColumn);

    QStringList getMatches(const QString &szFind, const NetworkAddressesEnum::Columns eColumn, const Qt::MatchFlags eMatchFlag = Qt::MatchStartsWith);

    inline QStandardItemModel *getModel() const { return tableModelAddresses; }

    void setValues(const QString &szAddressName, const QString &szServerIpV4, const QString &szServerPort);

protected:
    void pushButtonSaveAddressPushed(bool checked);

    void customContextMenuRequestedOnTableView(const QPoint &pos);

    void menuActionClickedConnect(bool bState);

private:
    void setupUi();
    void setupSignalsAndSlots();
    void loadSettings();
    void saveSettings();

    QLineEdit               *lineEditServerName{};
    QLineEdit               *lineEditServerIpV4{};
    QLineEdit               *lineEditServerPort{};

    QPushButton             *pushButtonSaveAddress{};

    NetworkAddressesModel   *tableModelAddresses{};
    StandardItemView        *tableViewAddresses{};

Q_SIGNALS:
    void aboutToHide();

    void connectionRequested(const QString &szAddressName);

//reimplemented
protected:
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
