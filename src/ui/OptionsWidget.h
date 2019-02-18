#pragma once

#include <QApplication>
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
    void saveSourceCodeLocations();

    QLabel                      *labelThemeChoice{};
    QComboBox                   *comboBoxSupportedLanguages{};
    QComboBox                   *comboBoxThemeChoice{};

    QComboBox                   *comboBoxCodeEditorNames{};
    QLineEdit                   *lineEditEditorLocation{};
    QPushButton                 *pushButtonEditorLocationPick{};
    QLineEdit                   *lineEditEditorHandling{};

    QPushButton                 *pushButtonSourceCodeLocationAdd{};
    QPushButton                 *pushButtonSourceCodeLocationRemove{};
    QListWidget                 *listWidgetSourceCodeLocations{};

    QDialogButtonBox            *buttonBoxCloseWindow{};

signals:
    void aboutToHide();

protected slots:
    void themeSelectionChanged(const QString &szNewTheme);

    void codeEditorLanguageChanged(const QString &szNewLanguage);
    void codeEditorSelectionChanged(const QString &szNewEditor);
    void codeEditorLocationChanged(const QString &szNewLocation);

    void buttonEditorLocationPickClicked(bool bState);
    void buttonEditorLocationPickResult(const QString &szFile);

    void buttonAddLocationClicked(bool bState);
    void buttonAddLocationResult(const QString &szFolder);
    void buttonRemoveLocationClicked(bool bState);

    void sourceLocationsListItemChanged(int nCurrentItem);

//reimplemented
protected:
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
