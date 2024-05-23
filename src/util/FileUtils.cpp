#include "FileUtils.h"
#include "application/GlobalConstants.h"

bool FileUtils::isFileTypeLog(const QString &szFilename)
{
    const QFileInfo myFileInfoLog(szFilename);

    bool bIsLog = false;

    if (myFileInfoLog.fileName().endsWith(GlobalConstants::FILE_EXTENSION_LOG) == true
        || myFileInfoLog.fileName().endsWith(GlobalConstants::FILE_EXTENSION_TXT) == true
        || myFileInfoLog.fileName().contains(GlobalConstants::FILE_EXTENSION_LOG + '.') == true ) { //ex: filename.log.3 (for rolling file logs)

        bIsLog = true;
    }

    return bIsLog;
}

QStringList FileUtils::getFileTypeFilterLog()
{
    QStringList szaFilters;
    szaFilters << '*' + GlobalConstants::FILE_EXTENSION_LOG
               << '*' + GlobalConstants::FILE_EXTENSION_TXT;

    return szaFilters;
}

QString FileUtils::getErrorOnlyLogAllowed()
{
    static const QString szOnlyLogAllowed = tr("Wrong file type. Allowed: ") + GlobalConstants::FILE_EXTENSION_LOG + ", " + GlobalConstants::FILE_EXTENSION_TXT;

    return szOnlyLogAllowed;
}
