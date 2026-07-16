#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace eb {

class IncomeRepository {
public:
    explicit IncomeRepository(QSqlDatabase db) : m_db(db) {}

    QVector<Income> query(const TransactionFilter &f) const;
    std::optional<Income> byId(qint64 id) const;
    qint64 insert(const Income &i);
    bool update(const Income &i);
    bool remove(qint64 id);
    bool restore(const Income &i);
    qint64 totalCents(const TransactionFilter &f) const;
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
