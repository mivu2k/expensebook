#pragma once
#include "models/Models.h"
#include "repositories/CategoryRepository.h"
#include "repositories/SubCategoryRepository.h"
#include <QObject>

namespace eb {

class AuditService;

class CategoryController : public QObject {
    Q_OBJECT
public:
    CategoryController(QSqlDatabase db, AuditService &audit, QObject *parent = nullptr);

    QVector<Category> categories(const QString &kind = QString()) const
    { return m_categories.all(kind); }
    QVector<SubCategory> subCategories(qint64 categoryId) const
    { return m_subCategories.byCategory(categoryId); }

    qint64 saveCategory(Category c);
    bool removeCategory(qint64 id);
    qint64 saveSubCategory(SubCategory s);
    bool removeSubCategory(qint64 id);
    QString errorMessage() const { return m_error; }

signals:
    void dataChanged();

private:
    CategoryRepository m_categories;
    SubCategoryRepository m_subCategories;
    AuditService &m_audit;
    QString m_error;
};

} // namespace eb
