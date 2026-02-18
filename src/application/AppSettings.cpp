#include "AppSettings.h"
#include "application/GlobalConstants.h"

void AppSettings::setValue(const AppSettings::SETTINGS eKey, const QVariant &myValue)
{
    getMySettings()->beginGroup(getKeyGroup(eKey));
    getMySettings()->setValue(getKeyString(eKey), myValue);
    getMySettings()->endGroup();
}

QVariant AppSettings::getValue(const AppSettings::SETTINGS eKey, const QVariant &myDefaultValue)
{
    getMySettings()->beginGroup(getKeyGroup(eKey));
    const QVariant myValue = getMySettings()->value(getKeyString(eKey), myDefaultValue);
    getMySettings()->endGroup();

    return myValue;
}

QVariant AppSettings::getDefaultValue(const AppSettings::SETTINGS eKey)
{
    switch (eKey) {
        case AppSettings::KEY_LOGGER_PATTERN:
            return QVariant("C++" + GlobalConstants::SEPARATOR_SETTINGS_LIST + QStringLiteral("%d %t [%c{1}] %p - %m"));

        case AppSettings::KEY_LOGGER_PATTERN_LIST:
            return QVariant("C++" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%d %t [%c{1}] %p - %m" + GlobalConstants::SEPARATOR_SETTINGS_LIST
                            + "C++ exported" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%d %t %c{1} %p %m" + GlobalConstants::SEPARATOR_SETTINGS_LIST
                            + "Java" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%d [%c{1}] %p - %m" + GlobalConstants::SEPARATOR_SETTINGS_LIST
                            + "Java exported" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%d %c{1} %p %m" + GlobalConstants::SEPARATOR_SETTINGS_LIST
                            + "Python" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%d - %c - %p - %m" + GlobalConstants::SEPARATOR_SETTINGS_LIST
                            + "Flat" + GlobalConstants::SEPARATOR_SETTINGS_LIST_2 + "%m");

        case AppSettings::KEY_FONT_SIZE:
            return QVariant(9);

        case AppSettings::KEY_ROW_HEIGHT_BIAS:
            return QVariant(0);

        case AppSettings::KEY_THEME_NAME:
            return QVariant("Dark Orange");

        case AppSettings::KEY_SERVER_IPv4:
        case AppSettings::KEY_SERVER_NAME:
        case AppSettings::KEY_SERVER_PORT:
        case AppSettings::KEY_WINDOW_POS_MAIN:
        case AppSettings::KEY_CODE_EDITOR_LOCATION:
        case AppSettings::KEY_CODE_EDITOR_NAME:
        case AppSettings::KEY_CODE_SOURCE_LANGUAGE:
        case AppSettings::KEY_CODE_SOURCE_LOCATION:
        case AppSettings::KEY_CODE_SOURCE_PROJECT:
        case AppSettings::KEY_FORMAT_EXPORTED_LOGS:
        case AppSettings::KEY_LOGGER_SERVER_ADDRESSES:
        case AppSettings::KEY_KEYWORDS_HIGHLIGHT:
        case AppSettings::COUNT_APP_SETTINGS:
            return QVariant();
    }
}

QSettings *AppSettings::getMySettings()
{
    static QSettings *mySettings = new QSettings(GlobalConstants::SETTINGS_FILE_NAME, QSettings::IniFormat);
    return mySettings;
}

QString AppSettings::getKeyGroup(const AppSettings::SETTINGS eKey)
{
    switch (eKey) {
        case AppSettings::KEY_FONT_SIZE:
        case AppSettings::KEY_LOGGER_PATTERN:
        case AppSettings::KEY_ROW_HEIGHT_BIAS:
        case AppSettings::KEY_SERVER_IPv4:
        case AppSettings::KEY_SERVER_NAME:
        case AppSettings::KEY_SERVER_PORT:
        case AppSettings::KEY_THEME_NAME:
        case AppSettings::KEY_WINDOW_POS_MAIN:
            return GlobalConstants::WIDGET_MAIN_WINDOW;

//        case AppSettings::KEY_CODE_EDITOR_HANDLING:
        case AppSettings::KEY_CODE_EDITOR_LOCATION:
        case AppSettings::KEY_CODE_EDITOR_NAME:
        case AppSettings::KEY_CODE_SOURCE_LANGUAGE:
        case AppSettings::KEY_CODE_SOURCE_LOCATION:
        case AppSettings::KEY_CODE_SOURCE_PROJECT:
            return GlobalConstants::WIDGET_SOURCE_CODE;

        case AppSettings::KEY_FORMAT_EXPORTED_LOGS:
        case AppSettings::KEY_LOGGER_PATTERN_LIST:
        case AppSettings::KEY_LOGGER_SERVER_ADDRESSES:
        case AppSettings::KEY_KEYWORDS_HIGHLIGHT:
            return GlobalConstants::APPLICATION_OPTIONS;

        case COUNT_APP_SETTINGS:
            return QString();
    }
}

QString AppSettings::getKeyString(const AppSettings::SETTINGS eKey)
{
    switch (eKey) {
        case AppSettings::KEY_FORMAT_EXPORTED_LOGS:
            return GlobalConstants::SETTINGS_FORMAT_EXPORTED_LOGS;

        case AppSettings::KEY_LOGGER_SERVER_ADDRESSES:
            return GlobalConstants::SETTINGS_LOGGER_SERVER_ADDRESSES;

        case AppSettings::KEY_KEYWORDS_HIGHLIGHT:
            return GlobalConstants::SETTINGS_KEYWORDS_HIGHLIGHT;

        case AppSettings::KEY_CODE_SOURCE_LANGUAGE:
            return GlobalConstants::SETTINGS_CODE_SOURCE_LANGUAGE;

        case AppSettings::KEY_CODE_SOURCE_LOCATION:
            return GlobalConstants::SETTINGS_CODE_SOURCE_LOCATION;

        case AppSettings::KEY_CODE_SOURCE_PROJECT:
            return GlobalConstants::SETTINGS_CODE_SOURCE_PROJECT;

//        case AppSettings::KEY_CODE_EDITOR_HANDLING:
//            return GlobalConstants::SETTINGS_CODE_EDITOR_HANDLING;

        case AppSettings::KEY_CODE_EDITOR_LOCATION:
            return GlobalConstants::SETTINGS_CODE_EDITOR_LOCATION;

        case AppSettings::KEY_CODE_EDITOR_NAME:
            return GlobalConstants::SETTINGS_CODE_EDITOR_NAME;

        case AppSettings::KEY_FONT_SIZE:
            return GlobalConstants::SETTINGS_FONT_SIZE;

        case AppSettings::KEY_LOGGER_PATTERN:
            return GlobalConstants::SETTINGS_LOGGER_PATTERN;

        case AppSettings::KEY_LOGGER_PATTERN_LIST:
            return GlobalConstants::SETTINGS_LOGGER_PATTERN_LIST;

        case AppSettings::KEY_ROW_HEIGHT_BIAS:
            return GlobalConstants::SETTINGS_ROW_HEIGHT_BIAS;

        case AppSettings::KEY_SERVER_IPv4:
            return GlobalConstants::SETTINGS_LABEL_SERVER_IPv4;

        case AppSettings::KEY_SERVER_NAME:
            return GlobalConstants::SETTINGS_LABEL_SERVER_NAME;

        case AppSettings::KEY_SERVER_PORT:
            return GlobalConstants::SETTINGS_LABEL_SERVER_PORT;

        case AppSettings::KEY_THEME_NAME:
            return GlobalConstants::SETTINGS_THEME_NAME;

        case AppSettings::KEY_WINDOW_POS_MAIN:
            return GlobalConstants::SETTINGS_WINDOW_POS_MAIN;

        case COUNT_APP_SETTINGS:
            return QString();
    }
}

