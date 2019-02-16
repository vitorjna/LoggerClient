#include "OptionsWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "application/SourceCodeHandler.h"
#include "util/FileUtils.h"

OptionsWidget::OptionsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();
    loadSettings();
}

OptionsWidget::~OptionsWidget()
{
}

void OptionsWidget::setupUi()
{
    int nCurrentRow = 0;

    QGridLayout *myMainLayout = new QGridLayout(this);
    myMainLayout->setContentsMargins(10, 10, 10, 10);
    myMainLayout->setSpacing(20);
    {
        labelThemeChoice = new QLabel(tr("Theme"), this);

        comboBoxThemeChoice = new QComboBox(this);
        comboBoxThemeChoice->addItems(loadThemeChoices());

//        mySearchTextLayout->addWidget(pushButtonNextResult);
    }
    myMainLayout->addWidget(labelThemeChoice,       nCurrentRow, 0);
    myMainLayout->addWidget(comboBoxThemeChoice,    nCurrentRow, 1);
    ++nCurrentRow;

    QGroupBox *sourceCodeGroup = new QGroupBox(tr("Log Classes and Source Code"));
    sourceCodeGroup->setFlat(true);
//    sourceCodeGroup->setStyleSheet("border-left:0;border-bottom:0;border-right:0;");

    QGridLayout *sourceCodeLayout = new QGridLayout(sourceCodeGroup);
    {
        QLabel *sourceCodeTypeLabel = new QLabel(this);
        sourceCodeTypeLabel->setText(tr("Code language"));

        comboBoxSupportedLanguages = new QComboBox(this);
        comboBoxSupportedLanguages->setMinimumContentsLength(5);
        comboBoxSupportedLanguages->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
        comboBoxSupportedLanguages->addItems(SourceCodeHandler::getSupportedLanguages());

        sourceCodeLayout->addWidget(sourceCodeTypeLabel,        0,  0);
        sourceCodeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 1);
        sourceCodeLayout->addWidget(comboBoxSupportedLanguages, 0,  2, 1, 2);

        //-------------------------------------------------
        QGroupBox *sourceCodeEditorGroup = new QGroupBox(tr("Editor to open"));
        QGridLayout *sourceCodeEditorLayout = new QGridLayout(sourceCodeEditorGroup);
        {
            QLabel *sourceCodeEditorLabel = new QLabel(this);
            sourceCodeEditorLabel->setText(tr("Application"));

            comboBoxCodeEditorNames = new QComboBox(this);
            comboBoxCodeEditorNames->addItems(SourceCodeHandler::getEditorNames().toList());

            sourceCodeEditorLayout->addWidget(sourceCodeEditorLabel,    0,  0);
            sourceCodeEditorLayout->addWidget(comboBoxCodeEditorNames,  0,  1);

            lineEditEditorLocation = new QLineEdit(this);
            lineEditEditorLocation->setPlaceholderText(tr("Editor location"));

            lineEditEditorHandling = new QLineEdit(this);
            lineEditEditorHandling->setPlaceholderText(tr("Arguments to open application"));
            lineEditEditorHandling->setEnabled(false);

            sourceCodeEditorLayout->addWidget(lineEditEditorLocation,   1,  0, 1, -1);
            sourceCodeEditorLayout->addWidget(lineEditEditorHandling,   2,  0, 1, -1);
        }
        sourceCodeLayout->addWidget(sourceCodeEditorGroup, 1, 0, 1, -1);

        //-------------------------------------------------
        QGroupBox *sourceCodeLocationsGroup = new QGroupBox(tr("Source code locations"));
        QGridLayout *sourceCodeLocationsLayout = new QGridLayout(sourceCodeLocationsGroup);
        {
            QListWidget *sourceCodeLocationsWidget = new QListWidget(this);
            sourceCodeLocationsWidget->addItems(SourceCodeHandler::getSourceCodeLocations());

            sourceCodeLocationsLayout->addWidget(sourceCodeLocationsWidget, 0, 0);
        }
        sourceCodeLayout->addWidget(sourceCodeLocationsGroup,  2,  0, 1, -1);

    }
    myMainLayout->addWidget(sourceCodeGroup,        nCurrentRow, 0, 1, -1);
    ++nCurrentRow;


    buttonBoxCloseWindow = new QDialogButtonBox(this);
    buttonBoxCloseWindow->setStandardButtons(QDialogButtonBox::Close);

    myMainLayout->addWidget(buttonBoxCloseWindow,   nCurrentRow, 0, 1, -1, Qt::AlignRight);


    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->resize(400, 500);

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));

    this->hide();
//    this->show();
}

void OptionsWidget::setupSignalsAndSlots()
{
    connect(comboBoxThemeChoice,        &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::themeSelectionChanged);

    connect(buttonBoxCloseWindow,       &QDialogButtonBox::rejected,
            this,                       &OptionsWidget::close);

    connect(comboBoxSupportedLanguages, &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::codeEditorLanguageChanged);

    connect(comboBoxCodeEditorNames,    &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::codeEditorSelectionChanged);

    connect(lineEditEditorLocation,     &QLineEdit::textEdited,
            this,                       &OptionsWidget::codeEditorLocationChanged);
}

void OptionsWidget::loadSettings()
{
    QString szThemeSelected = AppSettings::getValue(AppSettings::KEY_THEME_NAME, GlobalConstants::SETTINGS_THEME_DEFAULT).toString();
    comboBoxThemeChoice->setCurrentText(szThemeSelected);

    QString szCodeLanguage = AppSettings::getValue(AppSettings::KEY_SOURCE_LANGUAGE, SourceCodeHandler::getSupportedLanguages().at(0)).toString();
    comboBoxSupportedLanguages->setCurrentText(szCodeLanguage);

    QString szEditorName = AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_NAME, SourceCodeHandler::getEditorName(SourceCodeHandler::QtCreator)).toString();
    comboBoxCodeEditorNames->setCurrentText(szEditorName);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorHandling(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
}

QStringList OptionsWidget::loadThemeChoices()
{
    QDir myThemesDir(GlobalConstants::SETTINGS_FOLDER_THEMES);

    QStringList szaThemesAvailable = myThemesDir.entryList(QStringList("*" + GlobalConstants::FILE_EXTENSION_QSS), QDir::Files, QDir::Name | QDir::IgnoreCase);

    szaThemesAvailable.replaceInStrings(GlobalConstants::FILE_EXTENSION_QSS, QLatin1String(""));

    return szaThemesAvailable;
}

void OptionsWidget::themeSelectionChanged(const QString &szNewTheme)
{
    QString szThemeFilename(GlobalConstants::SETTINGS_FOLDER_THEMES
                            + "/"
                            + szNewTheme
                            + GlobalConstants::FILE_EXTENSION_QSS);

    if (QFileInfo::exists(szThemeFilename) == false) {
        return;
    }

    QFile fileStyleSheet(szThemeFilename);

    if (fileStyleSheet.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::Unbuffered) == true) {
        QTextStream streamStyleSheet(&fileStyleSheet);

        qApp->setStyleSheet(streamStyleSheet.readAll());

        fileStyleSheet.close();

    } else {
        qDebug() << "Could not load stylesheet " << szNewTheme;
    }

    AppSettings::setValue(AppSettings::KEY_THEME_NAME, szNewTheme);
}

void OptionsWidget::codeEditorLanguageChanged(const QString &szNewLanguage)
{
    AppSettings::setValue(AppSettings::KEY_SOURCE_LANGUAGE, szNewLanguage);
}

void OptionsWidget::codeEditorSelectionChanged(const QString &szNewEditor)
{
    AppSettings::setValue(AppSettings::KEY_CODE_EDITOR_NAME, szNewEditor);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorHandling(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
}

void OptionsWidget::codeEditorLocationChanged(const QString &szNewLocation)
{
    SourceCodeHandler::setEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex()), szNewLocation);
}

void OptionsWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

