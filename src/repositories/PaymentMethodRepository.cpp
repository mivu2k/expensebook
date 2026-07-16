#include "PaymentMethodRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

QVector<PaymentMethod> PaymentMethodRepository::all() const
{
    QVector<PaymentMethod> out;
    QSqlQuery q(m_db);
    if (!q.exec("SELECT id, name FROM PaymentMethods ORDER BY id")) {
        m_lastError = q.lastError().text();
        return out;
    }
    while (q.next())
        out.append({q.value(0).toLongLong(), q.value(1).toString()});
    return out;
}

qint64 PaymentMethodRepository::insert(const QString &name)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO PaymentMethods(name) VALUES(?)");
    q.addBindValue(name);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool PaymentMethodRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM PaymentMethods WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

} // namespace eb
