#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>

namespace eb {

class AuditLogRepository {
public:
    explicit AuditLogRepository(QSqlDatabase db) : m_db(db) {}

    bool insert(const QString &entity, qint64 entityId,
                const QString &action, const QString &details);
    QVector<AuditLog> recent(int limit = 200) const;
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
