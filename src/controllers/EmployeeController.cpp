#include "EmployeeController.h"
#include "services/AuditService.h"

namespace eb {

EmployeeController::EmployeeController(QSqlDatabase db, AuditService &audit,
                                       QObject *parent)
    : QObject(parent), m_repo(db), m_audit(audit)
{
}

qint64 EmployeeController::save(Employee e)
{
    e.name = e.name.trimmed();
    if (e.name.isEmpty()) { m_error = "Employee name is required."; return 0; }
    if (e.id == 0) {
        e.id = m_repo.insert(e);
        if (!e.id) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Employee", e.id, "insert", e.name);
    } else {
        if (!m_repo.update(e)) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Employee", e.id, "update", e.name);
    }
    emit dataChanged();
    return e.id;
}

bool EmployeeController::remove(qint64 id)
{
    if (!m_repo.remove(id)) { m_error = m_repo.lastError(); return false; }
    m_audit.log("Employee", id, "delete");
    emit dataChanged();
    return true;
}

} // namespace eb
