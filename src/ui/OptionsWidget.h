#pragma once

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QPushButton>
#include <QStyle>

class OptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsWidget(QWidget *parent = nullptr);
    ~OptionsWidget() override;

private:
    void setupUi();
    void setupSignalsAndSlots();
    void loadSettings();

    QStringList loadThemeChoices();

    QLabel                      *labelThemeChoice{};
    QComboBox                   *comboBoxSupportedLanguages{};
    QComboBox                   *comboBoxThemeChoice{};
    QComboBox                   *comboBoxCodeEditorNames{};
    QLineEdit                   *lineEditEditorLocation{};
    QLineEdit                   *lineEditEditorHandling{};

    QDialogButtonBox            *buttonBoxCloseWindow{};

signals:
    void aboutToHide();

protected slots:
    void themeSelectionChanged(const QString &szNewTheme);

    void codeEditorLanguageChanged(const QString &szNewLanguage);
    void codeEditorSelectionChanged(const QString &szNewEditor);
    void codeEditorLocationChanged(const QString &szNewLocation);


//reimplemented
protected:
    void closeEvent(QCloseEvent *event) override;

};
