#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>

namespace eb {

class PaymentMethodRepository {
public:
    explicit PaymentMethodRepository(QSqlDatabase db) : m_db(db) {}

    QVector<PaymentMethod> all() const;
    qint64 insert(const QString &name);
    bool remove(qint64 id);
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
