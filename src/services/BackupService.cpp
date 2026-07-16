#include "BackupService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

QString BackupService::createBackup(const QString &targetDir)
{
    if (!QDir().mkpath(targetDir)) {
        m_lastError = "Cannot create backup directory: " + targetDir;
        return {};
    }
    const QString file = QString("%1/expensebook-%2.db")
        .arg(targetDir,
             QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"));

    // VACUUM INTO produces a consistent snapshot even in WAL mode.
    QSqlQuery q(m_db);
    q.prepare("VACUUM INTO ?");
    q.addBindValue(file);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return {};
    }

    QSqlQuery rec(m_db);
    rec.prepare("INSERT INTO Backups(file_path) VALUES(?)");
    rec.addBindValue(file);
    rec.exec(); // history record is best-effort

    return file;
}

bool BackupService::restoreBackup(const QString &backupFile)
{
    if (!QFile::exists(backupFile)) {
        m_lastError = "Backup file not found: " + backupFile;
        return false;
    }
    // Close the live connection, replace the file, and let the caller
    // restart the app (which reopens and re-migrates).
    const QString conn = m_db.connectionName();
    m_db.close();

    const QString bak = m_dbFilePath + ".pre-restore";
    QFile::remove(bak);
    if (!QFile::rename(m_dbFilePath, bak)) {
        m_lastError = "Cannot move current database aside.";
        return false;
    }
    // Remove WAL sidecar files so the restored snapshot is authoritative.
    QFile::remove(m_dbFilePath + "-wal");
    QFile::remove(m_dbFilePath + "-shm");

    if (!QFile::copy(backupFile, m_dbFilePath)) {
        QFile::rename(bak, m_dbFilePath); // roll back
        m_lastError = "Copy failed; original database left untouched.";
        QSqlDatabase::database(conn).open();
        return false;
    }
    QFile::remove(bak);
    return true;
}

QVector<BackupInfo> BackupService::history() const
{
    QVector<BackupInfo> out;
    QSqlQuery q(m_db);
    if (q.exec("SELECT id, file_path, created_at FROM Backups ORDER BY id DESC"))
        while (q.next())
            out.append({q.value(0).toLongLong(), q.value(1).toString(),
                        q.value(2).toString()});
    return out;
}

} // namespace eb
