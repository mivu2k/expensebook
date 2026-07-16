#include "CategoryController.h"
#include "services/AuditService.h"

namespace eb {

CategoryController::CategoryController(QSqlDatabase db, AuditService &audit,
                                       QObject *parent)
    : QObject(parent), m_categories(db), m_subCategories(db), m_audit(audit)
{
}

qint64 CategoryController::saveCategory(Category c)
{
    c.name = c.name.trimmed();
    if (c.name.isEmpty()) { m_error = "Category name is required."; return 0; }
    if (c.id == 0) {
        c.id = m_categories.insert(c);
        if (!c.id) { m_error = m_categories.lastError(); return 0; }
        m_audit.log("Category", c.id, "insert", c.name);
    } else {
        if (!m_categories.update(c)) { m_error = m_categories.lastError(); return 0; }
        m_audit.log("Category", c.id, "update", c.name);
    }
    emit dataChanged();
    return c.id;
}

bool CategoryController::removeCategory(qint64 id)
{
    if (!m_categories.remove(id)) { m_error = m_categories.lastError(); return false; }
    m_audit.log("Category", id, "delete");
    emit dataChanged();
    return true;
}

qint64 CategoryController::saveSubCategory(SubCategory s)
{
    s.name = s.name.trimmed();
    if (s.name.isEmpty())     { m_error = "Sub-category name is required."; return 0; }
    if (s.categoryId <= 0)    { m_error = "Choose a parent category."; return 0; }
    if (s.id == 0) {
        s.id = m_subCategories.insert(s);
        if (!s.id) { m_error = m_subCategories.lastError(); return 0; }
        m_audit.log("SubCategory", s.id, "insert", s.name);
    } else {
        if (!m_subCategories.update(s)) { m_error = m_subCategories.lastError(); return 0; }
        m_audit.log("SubCategory", s.id, "update", s.name);
    }
    emit dataChanged();
    return s.id;
}

bool CategoryController::removeSubCategory(qint64 id)
{
    if (!m_subCategories.remove(id)) { m_error = m_subCategories.lastError(); return false; }
    m_audit.log("SubCategory", id, "delete");
    emit dataChanged();
    return true;
}

} // namespace eb
