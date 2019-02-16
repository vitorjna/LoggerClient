#include "SourceCodeHandler.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"

const QString SourceCodeHandler::szArgumentSourceFileName = QStringLiteral("%S");
const QString SourceCodeHandler::szArgumentSourceFileLine = QStringLiteral("%L");

const QVector<QString> SourceCodeHandler::szaEditorNames = initEditorNames();

QStringList SourceCodeHandler::getSupportedLanguages()
{
    QStringList szaSupportedLanguages;
    szaSupportedLanguages << QStringLiteral("C++")
                          << QStringLiteral("Java");

    return szaSupportedLanguages;
}

QStringList SourceCodeHandler::getSourceCodeLocations()
{
    QString szSourceLocations = AppSettings::getValue(AppSettings::KEY_SOURCE_LOCATION).toString();

    QStringList szaSourceFolders;

    if (szSourceLocations.isEmpty() == true) {
        return szaSourceFolders;
    }

    szaSourceFolders = szSourceLocations.split(GlobalConstants::SETTINGS_STRING_SEPARATOR);

    QString szSourceFolder4(QStringLiteral("C:\\Projects\\IntegraTEC_Ingenico\\src"));
    QString szSourceFolder5(QStringLiteral("C:\\Projects\\IntegraTEC_Core\\src"));
    QString szSourceFolder6(QStringLiteral("C:\\Projects\\IntegraTEC_CppDK\\src"));

    QString szSourceFolder7(QStringLiteral("C:\\git\\IntegraTEC_Ingenico\\src"));
    QString szSourceFolder8(QStringLiteral("C:\\git\\IntegraTEC_Core\\src"));
    QString szSourceFolder9(QStringLiteral("C:\\git\\IntegraTEC_CppDK\\src"));

    return szaSourceFolders;
}

QVector<QString> SourceCodeHandler::getEditorNames()
{
    return szaEditorNames; //TODO handle custom editors that have been saved
}

QString SourceCodeHandler::getEditorName(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    return szaEditorNames.at(eEditor);
}

QString SourceCodeHandler::getEditorLocation(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    QString szEditorLocations = AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_LOCATION).toString();

    if (szEditorLocations.isEmpty() == false) {
        QStringList szaEditorLocations = szEditorLocations.split(GlobalConstants::SETTINGS_STRING_SEPARATOR);

        if (szaEditorLocations.size() == COUNT_SOURCE_CODE_EDITORS) {
            return szaEditorLocations.at(eEditor);
        }
    }

    return getEditorLocationDefault(static_cast<SourceCodeEditors>(eEditor));
}

void SourceCodeHandler::setEditorLocation(const SourceCodeHandler::SourceCodeEditors eEditor, const QString &szEditorLocation)
{
    QString szEditorLocations = AppSettings::getValue(AppSettings::KEY_CODE_EDITOR_LOCATION).toString();

    QStringList szaEditorLocations = szEditorLocations.split(GlobalConstants::SETTINGS_STRING_SEPARATOR);

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

    AppSettings::setValue(AppSettings::KEY_CODE_EDITOR_LOCATION, szaEditorLocations.join(GlobalConstants::SETTINGS_STRING_SEPARATOR));
}

QString SourceCodeHandler::getEditorHandling(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    switch (eEditor) {
        case SourceCodeHandler::QtCreator:
            return QStringLiteral("-client ") + szArgumentSourceFileName + ':' + szArgumentSourceFileLine;

        case SourceCodeHandler::Eclipse:
            return QLatin1String("");

        case SourceCodeHandler::IngeDev:
            return QLatin1String("");

        case SourceCodeHandler::COUNT_SOURCE_CODE_EDITORS:
            return QLatin1String("");
    }
}

QString SourceCodeHandler::buildCallToEditor(const SourceCodeHandler::SourceCodeEditors eEditor, const QString &szFilenameFullPath, const QString &szLine)
{
    QString szHandling = getEditorHandling(eEditor);
    szHandling.replace(szArgumentSourceFileName, szFilenameFullPath);
    szHandling.replace(szArgumentSourceFileLine, szLine);

    QString szCall = getEditorLocation(eEditor);
    szCall.append(' ');
    szCall.append(szHandling);

    return szCall;
}

QVector<QString> SourceCodeHandler::initEditorNames()
{
    QVector<QString> szaSourceCodeEditors;
    szaSourceCodeEditors.resize(COUNT_SOURCE_CODE_EDITORS);

    szaSourceCodeEditors[QtCreator]  = QStringLiteral("QtCreator");
    szaSourceCodeEditors[Eclipse]    = QStringLiteral("Eclipse");
    szaSourceCodeEditors[IngeDev]    = QStringLiteral("IngeDev");

    return szaSourceCodeEditors;
}

QString SourceCodeHandler::getEditorLocationDefault(const SourceCodeHandler::SourceCodeEditors eEditor)
{
    switch (eEditor) {
        case SourceCodeHandler::QtCreator:
            return QStringLiteral("C:\\Qt\\Tools\\QtCreator\\bin\\qtcreator");

        case SourceCodeHandler::Eclipse:
            return QLatin1String("");

        case SourceCodeHandler::IngeDev:
            return QLatin1String("");

        case SourceCodeHandler::COUNT_SOURCE_CODE_EDITORS:
            return QLatin1String("");
    }
}
