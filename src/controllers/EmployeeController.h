#pragma once
#include "models/Models.h"
#include "repositories/EmployeeRepository.h"
#include <QObject>

namespace eb {

class AuditService;

class EmployeeController : public QObject {
    Q_OBJECT
public:
    EmployeeController(QSqlDatabase db, AuditService &audit, QObject *parent = nullptr);

    QVector<Employee> list(bool activeOnly = false) const { return m_repo.all(activeOnly); }
    qint64 save(Employee e);
    bool remove(qint64 id);
    QString errorMessage() const { return m_error; }

signals:
    void dataChanged();

private:
    EmployeeRepository m_repo;
    AuditService &m_audit;
    QString m_error;
};

} // namespace eb
