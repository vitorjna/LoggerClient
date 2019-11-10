#pragma once

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
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

#include "data/SourceCodeLocationsModel.h"
#include "view/StandardItemView.h"

class StandardItemView;

class SourceCodeLocationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SourceCodeLocationWidget(QWidget *parent = nullptr);
    ~SourceCodeLocationWidget() override = default;

    inline QStandardItemModel *getModel() const { return tableModelLocations; }
    inline StandardItemView *getView() const { return tableViewLocations; }

    void setValues(const QString &szProjectName, const QString &szSourceLocation);

    QStringList getProjectNames();

protected:
    void buttonSaveLocationPushed(bool checked);

    void buttonPickLocationPushed(bool bState);

private:
    void setupUi();
    void setupSignalsAndSlots();
    void loadSettings();
    void saveSettings();

    QLineEdit                   *lineEditProject;
    QLineEdit                   *lineEditSourceLocation;

    QPushButton                 *pushButtonPickLocation;
    QPushButton                 *pushButtonSaveLocation;

    SourceCodeLocationsModel    *tableModelLocations;
    StandardItemView            *tableViewLocations;

signals:
    void aboutToHide();

    void newProjectAdded(const QString &szProjectName);
    void projectListChanged();


//reimplemented
protected:
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};
