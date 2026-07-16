#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace eb {

class EmployeeRepository {
public:
    explicit EmployeeRepository(QSqlDatabase db) : m_db(db) {}

    QVector<Employee> all(bool activeOnly = false) const;
    std::optional<Employee> byId(qint64 id) const;
    qint64 insert(const Employee &e);        // returns new id, 0 on error
    bool update(const Employee &e);
    bool remove(qint64 id);
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
