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

#include "data/LoggerPatternsModel.h"

class StandardItemView;

class LoggerPatternManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoggerPatternManagerWidget(QWidget *parent = nullptr);
    ~LoggerPatternManagerWidget() override;

    QStringList getRow(const QString &szFind, const LoggerPatternsEnum::Columns eColumn);

    int getRowIndex(const QString &szFind, const LoggerPatternsEnum::Columns eColumn);

    QStringList getMatches(const QString &szFind, const LoggerPatternsEnum::Columns eColumn, const Qt::MatchFlags eMatchFlag = Qt::MatchStartsWith);

    inline QStandardItemModel *getModel() const { return tableModelPatterns; }

    void setValues(const QString &szPatternName, const QString &szPattern);

protected:
    void pushButtonSavePatternPushed(bool checked);

private:
    void setupUi();
    void setupSignalsAndSlots();
    void loadSettings();
    void saveSettings();

    QLineEdit               *lineEditPatternName{};
    QLineEdit               *lineEditPattern{};

    QPushButton             *pushButtonSavePattern{};

    LoggerPatternsModel     *tableModelPatterns{};
    StandardItemView        *tableViewPatterns{};

signals:
    void aboutToHide();

    void patternChangeRequested(const QString &szPatternName);

//reimplemented
protected:
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
