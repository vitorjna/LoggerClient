#include "SourceCodeHandler.h"
#include "application/AppSettings.h"

const QString SourceCodeHandler::LANGUAGE_NAME_CPP = QStringLiteral("C++");
const QString SourceCodeHandler::LANGUAGE_NAME_JAVA = QStringLiteral("Java");

const QString SourceCodeHandler::szArgumentSourceFileName = QStringLiteral("%s");
const QString SourceCodeHandler::szArgumentSourceFileLine = QStringLiteral("%l");

const QStringList SourceCodeHandler::szaEditorNames = initEditorNames();
const QStringList SourceCodeHandler::szaSupportedLanguages = initSupportedLanguages();


QStringList SourceCodeHandler::getSupportedLanguages()
{
    return szaSupportedLanguages;
}

QString SourceCodeHandler::getCurrentLanguage()
{
    return AppSettings::getValue(AppSettings::KEY_SOURCE_LANGUAGE, SourceCodeHandler::getSupportedLanguages().at(0)).toString();
}

void SourceCodeHandler::setCurrentLanguage(const QString &szCurrentLanguage)
{
    AppSettings::setValue(AppSettings::KEY_SOURCE_LANGUAGE, szCurrentLanguage);
}

QString SourceCodeHandler::getCurrentLanguageFileExtension()
{
    QString szCurrentLanguage = getCurrentLanguage();
    int nCurrentLanguage = getSupportedLanguages().indexOf(szCurrentLanguage);

    if (nCurrentLanguage != -1) {

        switch (static_cast<SourceCodeLanguages>(nCurrentLanguage)) {
            case SourceCodeHandler::Cpp:
                return GlobalConstants::FILE_EXTENSION_CPP;

            case SourceCodeHandler::Java:
                return GlobalConstants::FILE_EXTENSION_JAVA;

            case SourceCodeHandler::COUNT_SOURCE_CODE_LANGUAGES:
                return QLatin1String("");
        }

    } else {
        return QLatin1String("");
    }
}

QStringList SourceCodeHandler::getSourceCodeLocations()
{
    QString szSourceLocations = AppSettings::getValue(AppSettings::KEY_SOURCE_LOCATION).toString();

    QStringList szaSourceFolders;

    if (szSourceLocations.isEmpty() == true) {
        return szaSourceFolders;
    }

    szaSourceFolders = szSourceLocations.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

    return szaSourceFolders;
}

void SourceCodeHandler::setSourceCodeLocations(const QStringList &szaSourceFolders)
{
    AppSettings::setValue(AppSettings::KEY_SOURCE_LOCATION, szaSourceFolders.join(GlobalConstants::SEPARATOR_SETTINGS_LIST));
}

QStringList SourceCodeHandler::getEditorNames()
{
    return szaEditorNames;
}

QString SourceCodeHandler::getEditorName(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    return szaEditorNames.at(eEditor);
}

QString SourceCodeHandler::getEditorLocation(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    QString szEditorLocations = AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_LOCATION).toString();

    if (szEditorLocations.isEmpty() == false) {
        QStringList szaEditorLocations = szEditorLocations.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

        if (szaEditorLocations.size() == COUNT_SOURCE_CODE_EDITORS) {
            return szaEditorLocations.at(eEditor);
        }
    }

    return getEditorLocationDefault(static_cast<SourceCodeEditors>(eEditor));
}

void SourceCodeHandler::setEditorLocation(const SourceCodeHandler::SourceCodeEditors eEditor, const QString &szEditorLocation)
{
    QString szEditorLocations = AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_LOCATION).toString();

    QStringList szaEditorLocations = szEditorLocations.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

    if (szaEditorLocations.size() == COUNT_SOURCE_CODE_EDITORS) {
        szaEditorLocations.replace(eEditor, szEditorLocation);

    } else {
        szaEditorLocations.clear();

        for (int nEditor = 0; nEditor < COUNT_SOURCE_CODE_EDITORS; ++nEditor) {
            if (nEditor == eEditor) {
                szaEditorLocations << szEditorLocation;

            } else {
                szaEditorLocations << getEditorLocationDefault(static_cast<SourceCodeEditors>(nEditor));
            }
        }
    }

    AppSettings::setValue(AppSettings::KEY_CODE_EDITOR_LOCATION, szaEditorLocations.join(GlobalConstants::SEPARATOR_SETTINGS_LIST));
}

SourceCodeHandler::SourceCodeEditors SourceCodeHandler::getCurrentEditor()
{
    return static_cast<SourceCodeEditors>(szaEditorNames.indexOf(getCurrentEditorName()));
}

QString SourceCodeHandler::getCurrentEditorName()
{
    return AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_NAME, SourceCodeHandler::getEditorName(SourceCodeHandler::QtCreator)).toString();
}

void SourceCodeHandler::setCurrentEditor(const QString &szCurrentEditor)
{
    AppSettings::setValue(AppSettings::KEY_CODE_EDITOR_NAME, szCurrentEditor);
}

QString SourceCodeHandler::getEditorHandling(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    switch (eEditor) {
        case SourceCodeHandler::QtCreator:
            return QStringLiteral("-client ") + szArgumentSourceFileName + ':' + szArgumentSourceFileLine;

        case SourceCodeHandler::Eclipse:
            return QStringLiteral("--launcher.openFile ") + szArgumentSourceFileName + ':' + szArgumentSourceFileLine;

        case SourceCodeHandler::IngeDev:
            return QStringLiteral("--launcher.openFile ") + szArgumentSourceFileName + ':' + szArgumentSourceFileLine;

        case SourceCodeHandler::COUNT_SOURCE_CODE_EDITORS:
            return QLatin1String("");
    }
}

GlobalConstants::ErrorCode SourceCodeHandler::openFileInEditor(const SourceCodeHandler::SourceCodeEditors eEditor, const QString &szFilenameFullPath, const QString &szLine)
{
    QString szHandling = getEditorHandling(eEditor);
    szHandling.replace(szArgumentSourceFileName, szFilenameFullPath);
    szHandling.replace(szArgumentSourceFileLine, szLine);

    QString szCall = getEditorLocation(eEditor);
    szCall.append(' ');
    szCall.append(szHandling);

    int nResult = QProcess::execute(szCall);

    if (nResult < 0) {
        return GlobalConstants::ERROR;

    } else {
        return GlobalConstants::SUCCESS;
    }
}

QStringList SourceCodeHandler::initEditorNames()
{
    QStringList szaEditorNames;
    szaEditorNames.reserve(COUNT_SOURCE_CODE_EDITORS);

    for (int nIndex = 0; nIndex < COUNT_SOURCE_CODE_EDITORS; ++nIndex) {
        switch (static_cast<SourceCodeEditors>(nIndex)) {
            case SourceCodeHandler::QtCreator:
                szaEditorNames << QStringLiteral("QtCreator");
                break;

            case SourceCodeHandler::Eclipse:
                szaEditorNames << QStringLiteral("Eclipse");
                break;

            case SourceCodeHandler::IngeDev:
                szaEditorNames << QStringLiteral("IngeDev");
                break;

            case SourceCodeHandler::COUNT_SOURCE_CODE_EDITORS:
                break;
        }
    }

    return szaEditorNames;
}

QStringList SourceCodeHandler::initSupportedLanguages()
{
    QStringList szaSupportedLanguages;
    szaSupportedLanguages.reserve(COUNT_SOURCE_CODE_LANGUAGES);

    for (int nIndex = 0; nIndex < COUNT_SOURCE_CODE_LANGUAGES; ++nIndex) {
        switch (static_cast<SourceCodeLanguages>(nIndex)) {
            case SourceCodeHandler::Cpp:
                szaSupportedLanguages << LANGUAGE_NAME_CPP;
                break;

            case SourceCodeHandler::Java:
                szaSupportedLanguages << LANGUAGE_NAME_JAVA;
                break;

            case SourceCodeHandler::COUNT_SOURCE_CODE_LANGUAGES:
                break;
        }
    }

    return szaSupportedLanguages;
}

QString SourceCodeHandler::getEditorLocationDefault(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    switch (eEditor) {
        case SourceCodeHandler::QtCreator:
            return QStringLiteral("C:\\Qt\\Tools\\QtCreator\\bin\\qtcreator.exe");

        case SourceCodeHandler::Eclipse:
            return QLatin1String("");

        case SourceCodeHandler::IngeDev:
            return QLatin1String("");

        case SourceCodeHandler::COUNT_SOURCE_CODE_EDITORS:
            return QLatin1String("");
    }
}
