#pragma once

#include <QDebug>
#include <QFileInfo>
#include <QObject>

class FileUtils : public QObject
{
    Q_OBJECT

public:
    static bool isFileTypeLog(const QString &szFilename);

    static QStringList getFileTypeFilterLog();

    static QString getErrorOnlyLogAllowed();

};
