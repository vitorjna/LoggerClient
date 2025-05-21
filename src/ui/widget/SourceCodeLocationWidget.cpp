#include "SourceCodeLocationWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "ui/ToastNotificationWidget.h"
#include "util/NetworkUtils.h"
#include "view/StandardItemView.h"

SourceCodeLocationWidget::SourceCodeLocationWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();

    loadSettings();
}

void SourceCodeLocationWidget::setValues(const QString &szProjectName, const QString &szSourceLocation)
{
    lineEditProject->setText(szProjectName);
    lineEditSourceLocation->setText(szSourceLocation);
}

QStringList SourceCodeLocationWidget::getProjectNames()
{
    QStringList szaProjectNames;
    szaProjectNames.reserve(tableModelLocations->rowCount());

    for (int nRow = 0; nRow < tableModelLocations->rowCount(); ++nRow) {
        const QString szProjectName = tableModelLocations->item(nRow, SourceCodeLocationsEnum::COLUMN_PROJECT_NAME)->text();

        if (szaProjectNames.contains(szProjectName) == false) {
            szaProjectNames.append(szProjectName);
        }
    }

    return szaProjectNames;
}

void SourceCodeLocationWidget::buttonSaveLocationPushed(bool checked)
{
    Q_UNUSED(checked)

    const QString szProjectName     = lineEditProject->text();
    const QString szSourceLocation  = lineEditSourceLocation->text();

    if (szSourceLocation.isEmpty() == true) {
        ToastNotificationWidget::showMessage(this, tr("Please add source location"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    if (tableModelLocations->indexOf(szProjectName, SourceCodeLocationsEnum::COLUMN_PROJECT_NAME) == -1) {
        Q_EMIT newProjectAdded(szProjectName);
    }

    int nRow;

    while ((nRow = tableModelLocations->indexOf(szSourceLocation, SourceCodeLocationsEnum::COLUMN_SOURCE_PATH)) != -1) {
        //found entry with same path

        if (tableModelLocations->item(nRow, SourceCodeLocationsEnum::COLUMN_PROJECT_NAME)->text() == szProjectName) {
            //entry also has same project name
            ToastNotificationWidget::showMessage(this, tr("Combination Project/Source already exists"), ToastNotificationWidget::INFO, 2000);
            return;
        }
    }

    //NOTE The same location can be added to different project without being a global location, so this is not checked. Ex: adding a location to 2 projects, but not to a 3rd one

    QList<QStandardItem *> myTableRow;

    myTableRow.append(new QStandardItem(szProjectName));
    myTableRow.append(new QStandardItem(szSourceLocation));

    tableModelLocations->appendRow(myTableRow);

    saveSettings();
}

void SourceCodeLocationWidget::buttonPickLocationPushed(bool bState)
{
    Q_UNUSED(bState)

    QFileDialog *myFileDialog = new QFileDialog(this);
    myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    myFileDialog->setFileMode(QFileDialog::Directory);
    myFileDialog->setOption(QFileDialog::ShowDirsOnly, true);

    myFileDialog->open();

    QObject::connect(myFileDialog, &QFileDialog::fileSelected,
    this, [ myFileDialog, this ](const QString & szFolder) {
        lineEditSourceLocation->setText(szFolder);

        myFileDialog->deleteLater();
    }
                    );
}

void SourceCodeLocationWidget::setupUi()
{
    int nCurrentRow = 0;

    QGridLayout *myMainLayout = new QGridLayout(this);
    myMainLayout->setContentsMargins(10, 10, 10, 10);
    myMainLayout->setSpacing(20);

    QHBoxLayout *myEditsLayout = new QHBoxLayout();
    {
        lineEditProject = new QLineEdit(this);
        lineEditProject->setPlaceholderText(tr("Project"));
        lineEditProject->setToolTip(tr("Project Name"));

        lineEditSourceLocation = new QLineEdit(this);
        lineEditSourceLocation->setPlaceholderText(tr("Source path"));
        lineEditSourceLocation->setToolTip(tr("Source code path"));

        pushButtonPickLocation = new QPushButton(this);
        pushButtonPickLocation->setCheckable(false);
        pushButtonPickLocation->setText(tr("Pick path"));

        pushButtonSaveLocation = new QPushButton(this);
        pushButtonSaveLocation->setCheckable(false);
        pushButtonSaveLocation->setText(tr("Save"));

        connect(lineEditProject, SIGNAL(returnPressed()), pushButtonSaveLocation, SLOT(click()));
        connect(lineEditSourceLocation, SIGNAL(returnPressed()), pushButtonSaveLocation, SLOT(click()));


        myEditsLayout->addWidget(lineEditProject);
        myEditsLayout->addWidget(lineEditSourceLocation);
        myEditsLayout->addWidget(pushButtonPickLocation);
        myEditsLayout->addWidget(pushButtonSaveLocation);
    }

    {
        tableModelLocations = new SourceCodeLocationsModel(this);
        tableModelLocations->setAllowDuplicates(false);
//        tableModelLocations->setSortColumn(SourceCodeLocationsEnum::COLUMN_PROJECT_NAME);

        tableViewLocations = new StandardItemView(this);
        tableViewLocations->setAlternatingRowColors(true);
        tableViewLocations->setModel(tableModelLocations);
        tableViewLocations->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//        tableViewLocations->setSortingEnabled(true);
        tableViewLocations->setShowGrid(true);
        tableViewLocations->setContextMenuPolicy(Qt::CustomContextMenu);
        tableViewLocations->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableViewLocations->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableViewLocations->setWordWrap(false);
        tableViewLocations->setDragDropMode(QAbstractItemView::InternalMove); //TODO //allows reordering

        //Horizontal
        tableViewLocations->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewLocations->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewLocations->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableViewLocations->horizontalHeader()->setHighlightSections(false);
        tableViewLocations->horizontalHeader()->setStretchLastSection(true);
        //Vertical
        tableViewLocations->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewLocations->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewLocations->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        tableViewLocations->verticalHeader()->setVisible(false);
    }

    myMainLayout->addLayout(myEditsLayout, nCurrentRow, 0);
    ++nCurrentRow;
//    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
    myMainLayout->addWidget(tableViewLocations, nCurrentRow, 0, 2, -1);

    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
//    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
//    this->resize(800, 600); //TODO doesn't work with large font sizes

//    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));
//    this->setWindowTitle(tr("Source Code Location Manager"));

//    this->hide(); //using embed mode in OptionsWidget. May change in the future
}

void SourceCodeLocationWidget::setupSignalsAndSlots()
{
    connect(pushButtonPickLocation,                 &QPushButton::clicked,
            this,                                   &SourceCodeLocationWidget::buttonPickLocationPushed);

    connect(pushButtonSaveLocation,                 &QPushButton::clicked,
            this,                                   &SourceCodeLocationWidget::buttonSaveLocationPushed);
}

void SourceCodeLocationWidget::loadSettings()
{
    const QString szSourceLocations = AppSettings::getValue(AppSettings::KEY_CODE_SOURCE_LOCATION).toString();

    if (szSourceLocations.isEmpty() == false) {

        const QStringList szaSourceLocations = szSourceLocations.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

        if (szSourceLocations.contains(GlobalConstants::SEPARATOR_SETTINGS_LIST_2) == true) {

            QList<QStandardItem *> myTableRow;
            myTableRow.reserve(SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS);

            for (const QString &szSourceLocationRow : szaSourceLocations) {
                const QStringList szaSourceLocation = szSourceLocationRow.split(GlobalConstants::SEPARATOR_SETTINGS_LIST_2);

                if (szaSourceLocation.size() != SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS) {
                    ToastNotificationWidget::showMessage(this, tr("Could not parse source location: ") + szSourceLocationRow, ToastNotificationWidget::ERROR, 3000);
                    continue;
                }

                for (int nCol = 0; nCol < SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS; ++nCol) {
                    myTableRow.append(new QStandardItem(szaSourceLocation.at(nCol)));
                }

                tableModelLocations->appendRow(myTableRow);
                myTableRow.clear();
            }

        } else { //parse first data type, containing only source locations
            QList<QStandardItem *> myTableRow;
            myTableRow.reserve(2);

            for (int nRow = 0; nRow < szaSourceLocations.size(); ++nRow) {
                myTableRow.append(new QStandardItem());
                myTableRow.append(new QStandardItem(szaSourceLocations.at(nRow)));

                tableModelLocations->appendRow(myTableRow);
                myTableRow.clear();
            }
        }
    }
}

void SourceCodeLocationWidget::saveSettings()
{
    QStringList szaSourceLocations;
    szaSourceLocations.reserve(tableModelLocations->rowCount());

    QStringList szaSourceLocation;
    szaSourceLocation.reserve(SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS);

    for (int nRow = 0; nRow < tableModelLocations->rowCount(); ++nRow) {
        for (int nColumn = 0; nColumn < SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS; ++nColumn) {
            const QString szText = tableModelLocations->item(nRow, nColumn)->text();

            szaSourceLocation.append(szText);
        }

        szaSourceLocations.append(szaSourceLocation.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_2));
        szaSourceLocation.clear();
    }

    AppSettings::setValue(AppSettings::KEY_CODE_SOURCE_LOCATION, szaSourceLocations.join(GlobalConstants::SEPARATOR_SETTINGS_LIST));
}

void SourceCodeLocationWidget::hideEvent(QHideEvent *event)
{
    Q_EMIT aboutToHide();

    QWidget::hideEvent(event);

    saveSettings();

    lineEditProject->clear();
    lineEditSourceLocation->clear();
}

void SourceCodeLocationWidget::closeEvent(QCloseEvent *event)
{
    Q_EMIT aboutToHide();

    QWidget::closeEvent(event);
}
