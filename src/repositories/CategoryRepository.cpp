#include "CategoryRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

QVector<Category> CategoryRepository::all(const QString &kind) const
{
    QVector<Category> out;
    QSqlQuery q(m_db);
    if (kind.isEmpty()) {
        q.prepare("SELECT id, name, kind FROM Categories ORDER BY name");
    } else {
        q.prepare("SELECT id, name, kind FROM Categories "
                  "WHERE kind = ? OR kind = 'both' ORDER BY name");
        q.addBindValue(kind);
    }
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next())
        out.append({q.value(0).toLongLong(), q.value(1).toString(), q.value(2).toString()});
    return out;
}

qint64 CategoryRepository::insert(const Category &c)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Categories(name, kind) VALUES(?,?)");
    q.addBindValue(c.name); q.addBindValue(c.kind);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool CategoryRepository::update(const Category &c)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE Categories SET name=?, kind=? WHERE id=?");
    q.addBindValue(c.name); q.addBindValue(c.kind); q.addBindValue(c.id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool CategoryRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Categories WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

} // namespace eb
