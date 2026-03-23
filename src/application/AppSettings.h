#pragma once

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QThread>

class AppSettings : public QObject
{
    Q_OBJECT

public:
    enum SETTINGS {
        ///LoggerClientWidget
        KEY_FONT_SIZE,
        KEY_LOGGER_PATTERN,
        KEY_LOGGER_PATTERN_LIST,
        KEY_ROW_HEIGHT_BIAS,
        KEY_SERVER_IPv4,
        KEY_SERVER_NAME,
        KEY_SERVER_PORT,
        KEY_THEME_NAME,
        KEY_WINDOW_POS_MAIN,

        ///SourceLocations
//        KEY_CODE_EDITOR_HANDLING,
        KEY_CODE_EDITOR_LOCATION,
        KEY_CODE_EDITOR_NAME,
        KEY_CODE_SOURCE_LANGUAGE,
        KEY_CODE_SOURCE_LOCATION,
        KEY_CODE_SOURCE_PROJECT,

        ///Other Options
        KEY_FORMAT_EXPORTED_LOGS,
        KEY_FILE_RELOAD_INTERVAL,
        KEY_LOGGER_SERVER_ADDRESSES,
        KEY_KEYWORDS_HIGHLIGHT,

        COUNT_APP_SETTINGS
    };

    static void setValue(const AppSettings::SETTINGS eKey, const QVariant &myValue);

    static QVariant getValue(const AppSettings::SETTINGS eKey, const QVariant &myDefaultValue = QVariant());

    static QVariant getDefaultValue(const AppSettings::SETTINGS eKey);

private:
    static QSettings *getMySettings();

    static QString getKeyGroup(const AppSettings::SETTINGS eKey);

    static QString getKeyString(const AppSettings::SETTINGS eKey);

};


