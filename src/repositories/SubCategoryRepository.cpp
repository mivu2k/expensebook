#include "SubCategoryRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

static QVector<SubCategory> run(QSqlQuery &q, QString &err)
{
    QVector<SubCategory> out;
    if (!q.exec()) { err = q.lastError().text(); return out; }
    while (q.next())
        out.append({q.value(0).toLongLong(), q.value(1).toLongLong(), q.value(2).toString()});
    return out;
}

QVector<SubCategory> SubCategoryRepository::byCategory(qint64 categoryId) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, category_id, name FROM SubCategories "
              "WHERE category_id = ? ORDER BY name");
    q.addBindValue(categoryId);
    return run(q, m_lastError);
}

QVector<SubCategory> SubCategoryRepository::all() const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, category_id, name FROM SubCategories ORDER BY name");
    return run(q, m_lastError);
}

qint64 SubCategoryRepository::insert(const SubCategory &s)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO SubCategories(category_id, name) VALUES(?,?)");
    q.addBindValue(s.categoryId); q.addBindValue(s.name);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool SubCategoryRepository::update(const SubCategory &s)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE SubCategories SET category_id=?, name=? WHERE id=?");
    q.addBindValue(s.categoryId); q.addBindValue(s.name); q.addBindValue(s.id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool SubCategoryRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM SubCategories WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

} // namespace eb
