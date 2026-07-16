#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>

namespace eb {

class AttachmentRepository {
public:
    explicit AttachmentRepository(QSqlDatabase db) : m_db(db) {}

    QVector<Attachment> byExpense(qint64 expenseId, bool withData = false) const;
    QByteArray dataOf(qint64 id) const;
    qint64 insert(const Attachment &a);
    bool remove(qint64 id);
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
