#pragma once

#include <QString>

class GlobalConstants
{
public:
    static const QChar      SETTINGS_STRING_SEPARATOR;

    ///Settings Groups
    static const QString    WIDGET_MAIN_WINDOW;
    static const QString    WIDGET_SOURCE_CODE;

    static const QString    SETTINGS_FILE_NAME;
    static const QString    SETTINGS_FOLDER_THEMES;

    ///LoggerClientWidget
    static const QString    SETTINGS_LABEL_SERVER_IPv4;
    static const QString    SETTINGS_LABEL_SERVER_PORT;
    static const QString    SETTINGS_LOGGER_PATTERN;
    static const QString    SETTINGS_THEME_DEFAULT;
    static const QString    SETTINGS_THEME_NAME;
    static const QString    SETTINGS_WINDOW_POS_MAIN;

    ///SourceCodeHandler
//    static const QString    SETTINGS_CODE_EDITOR_HANDLING;
    static const QString    SETTINGS_CODE_EDITOR_LOCATION;
    static const QString    SETTINGS_CODE_EDITOR_NAME;
    static const QString    SETTINGS_SOURCE_LANGUAGE;
    static const QString    SETTINGS_SOURCE_LOCATION;

    static const QString    FILE_EXTENSION_CPP;
    static const QString    FILE_EXTENSION_JAVA;
    static const QString    FILE_EXTENSION_LOG;
    static const QString    FILE_EXTENSION_QSS;
    static const QString    FILE_EXTENSION_TXT;

};


