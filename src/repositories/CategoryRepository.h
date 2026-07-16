#pragma once
#include "models/Models.h"
#include <QSqlDatabase>
#include <QVector>

namespace eb {

class CategoryRepository {
public:
    explicit CategoryRepository(QSqlDatabase db) : m_db(db) {}

    QVector<Category> all(const QString &kind = QString()) const; // "" = all kinds
    qint64 insert(const Category &c);
    bool update(const Category &c);
    bool remove(qint64 id);     // cascades to sub-categories
    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};

} // namespace eb
