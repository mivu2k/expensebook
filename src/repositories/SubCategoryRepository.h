#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>

namespace eb {

class SubCategoryRepository {
public:
    explicit SubCategoryRepository(QSqlDatabase db) : m_db(db) {}

    QVector<SubCategory> byCategory(qint64 categoryId) const;
    QVector<SubCategory> all() const;
    qint64 insert(const SubCategory &s);
    bool update(const SubCategory &s);
    bool remove(qint64 id);
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
