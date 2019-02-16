#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

class SourceCodeHandler
{
public:
    enum SourceCodeEditors {
        QtCreator,
        Eclipse,
        IngeDev,
        COUNT_SOURCE_CODE_EDITORS
    };

    static QStringList getSupportedLanguages();

    static QStringList getSourceCodeLocations();

    static QVector<QString> getEditorNames();

    static QString getEditorName(const SourceCodeEditors eEditor);

    static QString getEditorLocation(const SourceCodeEditors eEditor);
    static void setEditorLocation(const SourceCodeEditors eEditor, const QString &szEditorLocation);

    static QString getEditorHandling(const SourceCodeEditors eEditor);

    static QString buildCallToEditor(const SourceCodeEditors eEditor, const QString &szFilenameFullPath, const QString &szLine);

private:
    static const QString szArgumentSourceFileName;
    static const QString szArgumentSourceFileLine;

    static const QVector<QString> szaEditorNames;

    static QVector<QString> initEditorNames();
    static QString getEditorLocationDefault(const SourceCodeHandler::SourceCodeEditors eEditor);

};

