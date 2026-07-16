#include "MigrationManager.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

namespace {
QString loadResource(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    return QString::fromUtf8(f.readAll());
}

// Migration list. Index 0 = schema version 1. Append only; never edit
// an entry that has shipped.
QStringList migrations()
{
    return {
        loadResource(":/sql/schema.sql"),
        // Future example:
        // "ALTER TABLE Expenses ADD COLUMN approved INTEGER NOT NULL DEFAULT 0;",
    };
}
} // namespace

int MigrationManager::currentVersion()
{
    QSqlQuery q(m_db);
    q.exec("PRAGMA user_version");
    return q.next() ? q.value(0).toInt() : 0;
}

bool MigrationManager::setVersion(int v)
{
    return QSqlQuery(m_db).exec(QStringLiteral("PRAGMA user_version = %1").arg(v));
}

bool MigrationManager::applyStatements(const QString &sqlBundle)
{
    // Strip line comments, then split on ';'. Statements in our bundles
    // never contain literal semicolons.
    QStringList lines;
    for (const QString &line : sqlBundle.split('\n'))
        if (!line.trimmed().startsWith("--"))
            lines << line;

    for (const QString &stmt : lines.join('\n').split(';')) {
        const QString sql = stmt.trimmed();
        if (sql.isEmpty())
            continue;
        QSqlQuery q(m_db);
        if (!q.exec(sql)) {
            m_lastError = q.lastError().text() + "\nStatement: " + sql;
            return false;
        }
    }
    return true;
}

bool MigrationManager::migrate()
{
    const QStringList all = migrations();
    for (int v = currentVersion(); v < all.size(); ++v) {
        if (!m_db.transaction()) {
            m_lastError = m_db.lastError().text();
            return false;
        }
        if (!applyStatements(all.at(v)) || !setVersion(v + 1)) {
            m_db.rollback();
            return false;
        }
        if (!m_db.commit()) {
            m_lastError = m_db.lastError().text();
            return false;
        }
    }
    return true;
}

} // namespace eb
