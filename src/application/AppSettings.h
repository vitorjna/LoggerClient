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
        KEY_LOGGER_PATTERN,
        KEY_SERVER_IPv4,
        KEY_SERVER_PORT,
        KEY_THEME_NAME,
        KEY_WINDOW_POS_MAIN,

        ///SourceLocations
//        KEY_CODE_EDITOR_HANDLING,
        KEY_CODE_EDITOR_LOCATION,
        KEY_CODE_EDITOR_NAME,
        KEY_SOURCE_LANGUAGE,
        KEY_SOURCE_LOCATION,

        COUNT_APP_SETTINGS
    };

    static void setValue(const AppSettings::SETTINGS eKey, const QVariant &myValue);

    static QVariant getValue(const AppSettings::SETTINGS eKey, const QVariant &myDefaultValue = QVariant());

private:
    static QSettings *mySettings;

    static QSettings *initQSettings();

    static QString getKeyGroup(const AppSettings::SETTINGS eKey);

    static QString getKeyString(const AppSettings::SETTINGS eKey);

};


