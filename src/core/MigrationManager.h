#pragma once
#include <QSqlDatabase>
#include <QString>

namespace eb {

/// Versioned, forward-only schema migrations.
/// Each migration runs in its own transaction; the applied version is
/// recorded with PRAGMA user_version so upgrades are idempotent.
class MigrationManager {
public:
    explicit MigrationManager(QSqlDatabase db) : m_db(db) {}

    bool migrate();
    QString lastError() const { return m_lastError; }

private:
    int currentVersion();
    bool setVersion(int v);
    bool applyStatements(const QString &sqlBundle);

    QSqlDatabase m_db;
    QString m_lastError;
};

} // namespace eb
