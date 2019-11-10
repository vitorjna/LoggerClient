#pragma once

#include <QString>

class GlobalConstants
{
public:
    enum ErrorCode {
        SUCCESS,
        ERROR,
        COUNT_ERROR_CODE
    };

    static const QString    SEPARATOR_SETTINGS_LIST;
    static const QString    SEPARATOR_SETTINGS_LIST_2;
    static const QChar      SEPARATOR_EXPORTED_TEXT_LINE;
    static const QChar      SEPARATOR_EXPORTED_TEXT_COLUMN;

    ///Settings Groups
    static const QString    WIDGET_MAIN_WINDOW;
    static const QString    WIDGET_SOURCE_CODE;
    static const QString    APPLICATION_OPTIONS;

    static const QString    SETTINGS_FILE_NAME;
    static const QString    SETTINGS_FOLDER_THEMES;

    ///LoggerClientWidget
    static const QString    SETTINGS_FONT_SIZE;
    static const QString    SETTINGS_LABEL_SERVER_IPv4;
    static const QString    SETTINGS_LABEL_SERVER_NAME;
    static const QString    SETTINGS_LABEL_SERVER_PORT;
    static const QString    SETTINGS_LOGGER_PATTERN;
    static const QString    SETTINGS_LOGGER_PATTERN_LIST;
    static const QString    SETTINGS_ROW_HEIGHT_BIAS;
    static const QString    SETTINGS_THEME_NAME;
    static const QString    SETTINGS_WINDOW_POS_MAIN;

    ///SourceCodeHandler
//    static const QString    SETTINGS_CODE_EDITOR_HANDLING;
    static const QString    SETTINGS_CODE_EDITOR_LOCATION;
    static const QString    SETTINGS_CODE_EDITOR_NAME;
    static const QString    SETTINGS_CODE_SOURCE_LANGUAGE;
    static const QString    SETTINGS_CODE_SOURCE_LOCATION;
    static const QString    SETTINGS_CODE_SOURCE_PROJECT;

    ///Other Options
    static const QString    SETTINGS_FORMAT_EXPORTED_LOGS;
    static const QString    SETTINGS_LOGGER_SERVER_ADDRESSES;

    static const QString    FILE_EXTENSION_CPP;
    static const QString    FILE_EXTENSION_JAVA;
    static const QString    FILE_EXTENSION_LOG;
    static const QString    FILE_EXTENSION_QSS;
    static const QString    FILE_EXTENSION_TXT;

};


