#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace eb {

class ExpenseRepository {
public:
    explicit ExpenseRepository(QSqlDatabase db) : m_db(db) {}

    QVector<Expense> query(const TransactionFilter &f) const;
    std::optional<Expense> byId(qint64 id) const;
    qint64 insert(const Expense &e);
    bool update(const Expense &e);
    bool remove(qint64 id);
    /// Re-inserts a previously deleted expense keeping its original id
    /// (used by undo-delete).
    bool restore(const Expense &e);
    qint64 totalCents(const TransactionFilter &f) const;
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
