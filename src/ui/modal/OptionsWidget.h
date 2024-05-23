#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>

#include "ui/widget/SourceCodeLocationWidget.h"

class OptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsWidget(QWidget *parent = nullptr);
    ~OptionsWidget() override;

    void fontSizeChange(int nAmount);

    void rowHeightBiasChange(int nAmount);

private:
    void setupUi();
    void setupSignalsAndSlots();
    void loadSettings();

    QStringList loadThemeChoices() const;

    QComboBox                   *comboBoxThemeChoice{};
    QSpinBox                    *spinBoxFontSize{};
    QSpinBox                    *spinBoxRowHeightBias{};

    QCheckBox                   *checkBoxFormatExportedLogs{};

    QComboBox                   *comboBoxSupportedLanguages{};

    QComboBox                   *comboBoxCodeEditorNames{};
    QLineEdit                   *lineEditEditorLocation{};
    QPushButton                 *pushButtonEditorLocationPick{};
    QLineEdit                   *lineEditEditorHandling{};

    QComboBox                   *comboBoxActiveProjectName{};
    SourceCodeLocationWidget    *mySourceCodeLocationWidget{};

    QDialogButtonBox            *buttonBoxCloseWindow{};

signals:
    void aboutToHide();
    void fontSizeChanged(const int nValue);
    void rowHeightBiasChanged(const int nValue);

protected slots:
    void themeSelectionChanged(const QString &szNewTheme) const;

    void fontSizeChangedSlot(const int nValue);
    void rowHeightBiasChangedSlot(const int nValue);

    void formatExportedLogsChanged(const int nState) const;

    void codeEditorLanguageChanged(const QString &szNewLanguage) const;
    void codeEditorSelectionChanged(const QString &szNewEditor);
    void codeEditorLocationChanged(const QString &szNewLocation);

    void activeProjectNameSelectionChanged(const QString &szProjectName) const;

    void buttonEditorLocationPickClicked(bool bState);

//reimplemented
protected:
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
