#include "GlobalConstants.h"

const QString   GlobalConstants::SEPARATOR_SETTINGS_LIST            = QStringLiteral("<@>");
const QString   GlobalConstants::SEPARATOR_SETTINGS_LIST_2          = QStringLiteral("<@@>");
const QChar     GlobalConstants::SEPARATOR_EXPORTED_TEXT_LINE       = '\n';
const QChar     GlobalConstants::SEPARATOR_EXPORTED_TEXT_COLUMN     = ' ';

///Settings Groups
const QString   GlobalConstants::WIDGET_MAIN_WINDOW                 = QStringLiteral("LoggerClientWidget");
const QString   GlobalConstants::WIDGET_SOURCE_CODE                 = QStringLiteral("SourceCodeHandler");
const QString   GlobalConstants::APPLICATION_OPTIONS                = QStringLiteral("AppOptions");

const QString   GlobalConstants::SETTINGS_FILE_NAME                 = QStringLiteral("./LoggerClient.ini");
const QString   GlobalConstants::SETTINGS_FOLDER_THEMES             = QStringLiteral("./themes");

///LoggerClientWidget
const QString   GlobalConstants::SETTINGS_FONT_SIZE                 = QStringLiteral("FontSize");
const QString   GlobalConstants::SETTINGS_LABEL_SERVER_IPv4         = QStringLiteral("ServerIPv4");
const QString   GlobalConstants::SETTINGS_LABEL_SERVER_NAME         = QStringLiteral("ServerName");
const QString   GlobalConstants::SETTINGS_LABEL_SERVER_PORT         = QStringLiteral("ServerPort");
const QString   GlobalConstants::SETTINGS_LOGGER_PATTERN            = QStringLiteral("LoggerPattern");
const QString   GlobalConstants::SETTINGS_LOGGER_PATTERN_LIST       = QStringLiteral("LoggerPatternList");
const QString   GlobalConstants::SETTINGS_ROW_HEIGHT_BIAS           = QStringLiteral("RowHeightBias");
const QString   GlobalConstants::SETTINGS_THEME_NAME                = QStringLiteral("MainTheme");
const QString   GlobalConstants::SETTINGS_WINDOW_POS_MAIN           = QStringLiteral("PosMainWindow");

///SourceLocations
//const QString   GlobalConstants::SETTINGS_CODE_EDITOR_HANDLING      = QStringLiteral("EditorHandling");
const QString   GlobalConstants::SETTINGS_CODE_EDITOR_LOCATION      = QStringLiteral("EditorLocation");
const QString   GlobalConstants::SETTINGS_CODE_EDITOR_NAME          = QStringLiteral("EditorName");
const QString   GlobalConstants::SETTINGS_CODE_SOURCE_LANGUAGE      = QStringLiteral("SourceLanguage");
const QString   GlobalConstants::SETTINGS_CODE_SOURCE_LOCATION      = QStringLiteral("SourceLocation");
const QString   GlobalConstants::SETTINGS_CODE_SOURCE_PROJECT       = QStringLiteral("CurrentProject");

///Other Options
const QString   GlobalConstants::SETTINGS_FORMAT_EXPORTED_LOGS      = QStringLiteral("FormatExportedLogs");
const QString   GlobalConstants::SETTINGS_LOGGER_SERVER_ADDRESSES   = QStringLiteral("LoggerServerAddresses");

const QString   GlobalConstants::FILE_EXTENSION_CPP                 = QStringLiteral(".cpp");
const QString   GlobalConstants::FILE_EXTENSION_JAVA                = QStringLiteral(".java");
const QString   GlobalConstants::FILE_EXTENSION_LOG                 = QStringLiteral(".log");
const QString   GlobalConstants::FILE_EXTENSION_QSS                 = QStringLiteral(".qss");
const QString   GlobalConstants::FILE_EXTENSION_TXT                 = QStringLiteral(".txt");
