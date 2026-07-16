#pragma once
#include <QSqlDatabase>
#include <QString>

namespace eb {

/// Owns the application's single SQLite connection.
/// Opens (or creates) the .db file and runs pending migrations.
class Database {
public:
    /// Opens the database at @p filePath. Returns false and sets
    /// lastError() on failure. Safe to call once at startup.
    bool open(const QString &filePath);
    void close();

    QSqlDatabase connection() const;
    QString filePath() const { return m_filePath; }
    QString lastError() const { return m_lastError; }

private:
    QString m_filePath;
    QString m_lastError;
};

} // namespace eb
