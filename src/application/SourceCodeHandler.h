#pragma once

#include <QList>
#include <QString>
#include <QProcess>
#include <QStringList>

class SourceCodeHandler
{
public:
    enum SourceCodeLanguages {
        Cpp,
        Java,
        COUNT_SOURCE_CODE_LANGUAGES
    };

    enum SourceCodeEditors {
        QtCreator,
        Eclipse,
        IngeDev,
        COUNT_SOURCE_CODE_EDITORS
    };

    static const QString LANGUAGE_NAME_CPP;
    static const QString LANGUAGE_NAME_JAVA;

    static QStringList getSupportedLanguages();

    static QString getCurrentLanguage();
    static void setCurrentLanguage(const QString &szCurrentLanguage);

    static QString getCurrentLanguageFileExtension();

    static QStringList getSourceCodeLocations();
    static void setSourceCodeLocations(const QStringList &szaSourceFolders);

    static QStringList getEditorNames();

    static QString getEditorName(const SourceCodeEditors eEditor);

    static QString getEditorLocation(const SourceCodeEditors eEditor);
    static void setEditorLocation(const SourceCodeEditors eEditor, const QString &szEditorLocation);

    static SourceCodeEditors getCurrentEditor();
    static QString getCurrentEditorName();
    static void setCurrentEditor(const QString &szCurrentEditor);

    static QString getEditorHandling(const SourceCodeEditors eEditor);

    static void openFileInEditor(const SourceCodeEditors eEditor, const QString &szFilenameFullPath, const QString &szLine);

private:
    static const QString szArgumentSourceFileName;
    static const QString szArgumentSourceFileLine;

    static const QStringList szaEditorNames;
    static const QStringList szaSupportedLanguages;

    static QStringList initEditorNames();
    static QStringList initSupportedLanguages();
    static QString getEditorLocationDefault(const SourceCodeHandler::SourceCodeEditors eEditor);

};

