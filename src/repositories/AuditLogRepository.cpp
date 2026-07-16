#include "AuditLogRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

bool AuditLogRepository::insert(const QString &entity, qint64 entityId,
                                const QString &action, const QString &details)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO AuditLogs(entity, entity_id, action, details) VALUES(?,?,?,?)");
    q.addBindValue(entity); q.addBindValue(entityId);
    q.addBindValue(action); q.addBindValue(details);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

QVector<AuditLog> AuditLogRepository::recent(int limit) const
{
    QVector<AuditLog> out;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, entity, entity_id, action, details, created_at "
              "FROM AuditLogs ORDER BY id DESC LIMIT ?");
    q.addBindValue(limit);
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next()) {
        out.append({q.value(0).toLongLong(), q.value(1).toString(),
                    q.value(2).toLongLong(), q.value(3).toString(),
                    q.value(4).toString(), q.value(5).toString()});
    }
    return out;
}

} // namespace eb
