#include "EmployeeRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

static Employee fromRecord(const QSqlQuery &q)
{
    Employee e;
    e.id = q.value("id").toLongLong();
    e.name = q.value("name").toString();
    e.designation = q.value("designation").toString();
    e.phone = q.value("phone").toString();
    e.email = q.value("email").toString();
    e.active = q.value("active").toBool();
    return e;
}

QVector<Employee> EmployeeRepository::all(bool activeOnly) const
{
    QVector<Employee> out;
    QSqlQuery q(m_db);
    q.prepare(QString("SELECT * FROM Employees %1 ORDER BY name")
                  .arg(activeOnly ? "WHERE active = 1" : ""));
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next()) out.append(fromRecord(q));
    return out;
}

std::optional<Employee> EmployeeRepository::byId(qint64 id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM Employees WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) return fromRecord(q);
    m_lastError = q.lastError().text();
    return std::nullopt;
}

qint64 EmployeeRepository::insert(const Employee &e)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Employees(name, designation, phone, email, active) "
              "VALUES(?,?,?,?,?)");
    q.addBindValue(e.name); q.addBindValue(e.designation);
    q.addBindValue(e.phone); q.addBindValue(e.email);
    q.addBindValue(e.active ? 1 : 0);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool EmployeeRepository::update(const Employee &e)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE Employees SET name=?, designation=?, phone=?, email=?, active=? "
              "WHERE id=?");
    q.addBindValue(e.name); q.addBindValue(e.designation);
    q.addBindValue(e.phone); q.addBindValue(e.email);
    q.addBindValue(e.active ? 1 : 0); q.addBindValue(e.id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool EmployeeRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Employees WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

} // namespace eb
