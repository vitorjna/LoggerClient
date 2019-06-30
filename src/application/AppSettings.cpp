#include "AppSettings.h"
#include "application/GlobalConstants.h"

QSettings *AppSettings::mySettings = AppSettings::initQSettings();

void AppSettings::setValue(const AppSettings::SETTINGS eKey, const QVariant &myValue)
{
    mySettings->beginGroup(getKeyGroup(eKey));
    mySettings->setValue(getKeyString(eKey), myValue);
    mySettings->endGroup();
}

QVariant AppSettings::getValue(const AppSettings::SETTINGS eKey, const QVariant &myDefaultValue)
{
    mySettings->beginGroup(getKeyGroup(eKey));
    QVariant myValue = mySettings->value(getKeyString(eKey), myDefaultValue);
    mySettings->endGroup();

    return myValue;
}

QSettings *AppSettings::initQSettings()
{
    QSettings *mySettings = new QSettings(GlobalConstants::SETTINGS_FILE_NAME, QSettings::IniFormat);
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
        case AppSettings::KEY_SOURCE_LANGUAGE:
        case AppSettings::KEY_SOURCE_LOCATION:
            return GlobalConstants::WIDGET_SOURCE_CODE;

        case AppSettings::KEY_FORMAT_EXPORTED_LOGS:
        case AppSettings::KEY_LOGGER_SERVER_ADDRESSES:
            return GlobalConstants::APPLICATION_OPTIONS;

        case COUNT_APP_SETTINGS:
            return QLatin1String("");
    }
}

QString AppSettings::getKeyString(const AppSettings::SETTINGS eKey)
{
    switch (eKey) {
        case AppSettings::KEY_FORMAT_EXPORTED_LOGS:
            return GlobalConstants::SETTINGS_FORMAT_EXPORTED_LOGS;

        case AppSettings::KEY_LOGGER_SERVER_ADDRESSES:
            return GlobalConstants::SETTINGS_LOGGER_SERVER_ADDRESSES;

        case AppSettings::KEY_SOURCE_LANGUAGE:
            return GlobalConstants::SETTINGS_SOURCE_LANGUAGE;

        case AppSettings::KEY_SOURCE_LOCATION:
            return GlobalConstants::SETTINGS_SOURCE_LOCATION;

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
            return QLatin1String("");
    }
}

