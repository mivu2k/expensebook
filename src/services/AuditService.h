#pragma once
#include "repositories/AuditLogRepository.h"

namespace eb {

/// Thin façade over AuditLogRepository so controllers depend on a
/// service interface rather than the repository directly.
class AuditService {
public:
    explicit AuditService(QSqlDatabase db) : m_repo(db) {}

    void log(const QString &entity, qint64 id, const QString &action,
             const QString &details = QString())
    {
        m_repo.insert(entity, id, action, details);
    }

    QVector<AuditLog> recent(int limit = 200) const { return m_repo.recent(limit); }

private:
    AuditLogRepository m_repo;
};

} // namespace eb
