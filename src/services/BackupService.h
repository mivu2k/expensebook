#pragma once
#include <QSqlDatabase>
#include <QString>
#include <QVector>

namespace eb {

struct BackupInfo {
    qint64 id;
    QString filePath;
    QString createdAt;
};

/// Creates timestamped copies of the SQLite file and restores them.
/// Uses SQLite's VACUUM INTO for a consistent snapshot while the
/// database is open.
class BackupService {
public:
    BackupService(QSqlDatabase db, QString dbFilePath)
        : m_db(db), m_dbFilePath(std::move(dbFilePath)) {}

    /// Returns the path of the new backup file, or empty on failure.
    QString createBackup(const QString &targetDir);
    /// Restores @p backupFile over the live database. The app must be
    /// restarted afterwards; the caller handles that.
    bool restoreBackup(const QString &backupFile);
    QVector<BackupInfo> history() const;
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    QString m_dbFilePath;
    QString m_lastError;
};

} // namespace eb
