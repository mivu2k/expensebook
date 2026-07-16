#include "Database.h"
#include "MigrationManager.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

static const char *kConnectionName = "eb_main";

bool Database::open(const QString &filePath)
{
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QSqlDatabase db = QSqlDatabase::contains(kConnectionName)
        ? QSqlDatabase::database(kConnectionName)
        : QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    db.setDatabaseName(filePath);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }
    m_filePath = filePath;

    {
        // Scoped: an open PRAGMA result set would block the migration
        // transaction's commit.
        QSqlQuery pragma(db);
        pragma.exec("PRAGMA foreign_keys = ON");
        pragma.finish();
        pragma.exec("PRAGMA journal_mode = WAL");
        pragma.finish();
    }

    MigrationManager migrations(db);
    if (!migrations.migrate()) {
        m_lastError = migrations.lastError();
        return false;
    }
    return true;
}

void Database::close()
{
    if (QSqlDatabase::contains(kConnectionName)) {
        QSqlDatabase::database(kConnectionName).close();
        QSqlDatabase::removeDatabase(kConnectionName);
    }
}

QSqlDatabase Database::connection() const
{
    return QSqlDatabase::database(kConnectionName);
}

} // namespace eb
