#pragma once

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include "ui/element/LineEditWidget.h"
#include "ui/modal/LoggerPatternManagerWidget.h"

class LoggerPatternWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoggerPatternWidget(QWidget *parent = nullptr);
    ~LoggerPatternWidget() override;

    void setPatternName(const QString &szPatternName);
    inline QString getPatternName() {
        return comboBoxLoggerPatternName->currentText();
    }

    void setPattern(const QString &szPattern);
    inline QString getPattern() {
        return comboBoxLoggerPattern->currentText();
    }

protected:
    void loadCurrentPattern();
    void saveCurrentPattern();

private:
    void setupUi();
    void setupSignalsAndSlots();

    QLabel                          *labelLoggerPattern{};
    QComboBox                       *comboBoxLoggerPatternName{};
    QComboBox                       *comboBoxLoggerPattern{};

    QPushButton                     *buttonManagePatterns{};

    LoggerPatternManagerWidget      *myLoggerPatternManagerWidget{};

protected slots:
    void loggerPatternNameChanged(const QString &szPatternName);
    void loggerPatternChanged(const QString &szPattern);
    void buttonManagePatternsToggled(bool bButtonState);

    void patternChangeRequested(const QString &szPattern);

signals:
    void loggerPatternChangedSignal(QString);

    //reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};

