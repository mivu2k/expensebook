#include "ExpenseRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

namespace {

// Builds the shared WHERE clause for filtered queries. Bind values are
// appended to @p binds in the same order as the '?' placeholders.
QString whereClause(const TransactionFilter &f, QVariantList &binds)
{
    QStringList w;
    if (f.from.isValid()) { w << "e.date >= ?"; binds << f.from.toString(Qt::ISODate); }
    if (f.to.isValid())   { w << "e.date <= ?"; binds << f.to.toString(Qt::ISODate); }
    if (f.employeeId)      { w << "e.employee_id = ?";       binds << f.employeeId; }
    if (f.categoryId)      { w << "e.category_id = ?";       binds << f.categoryId; }
    if (f.subCategoryId)   { w << "e.subcategory_id = ?";    binds << f.subCategoryId; }
    if (f.paymentMethodId) { w << "e.payment_method_id = ?"; binds << f.paymentMethodId; }
    if (f.minCents >= 0)   { w << "e.amount >= ?";           binds << f.minCents; }
    if (f.maxCents >= 0)   { w << "e.amount <= ?";           binds << f.maxCents; }
    if (!f.text.isEmpty()) {
        w << "(e.description LIKE ? OR e.reference_no LIKE ? OR e.remarks LIKE ?)";
        const QString like = "%" + f.text + "%";
        binds << like << like << like;
    }
    return w.isEmpty() ? "" : "WHERE " + w.join(" AND ");
}

Expense fromRecord(const QSqlQuery &q)
{
    Expense e;
    e.id = q.value("id").toLongLong();
    e.date = QDate::fromString(q.value("date").toString(), Qt::ISODate);
    e.employeeId = q.value("employee_id").toLongLong();
    e.description = q.value("description").toString();
    e.categoryId = q.value("category_id").toLongLong();
    e.subCategoryId = q.value("subcategory_id").toLongLong();
    e.amountCents = q.value("amount").toLongLong();
    e.paymentMethodId = q.value("payment_method_id").toLongLong();
    e.referenceNo = q.value("reference_no").toString();
    e.remarks = q.value("remarks").toString();
    e.employeeName = q.value("employee_name").toString();
    e.categoryName = q.value("category_name").toString();
    e.subCategoryName = q.value("subcategory_name").toString();
    e.paymentMethodName = q.value("payment_method_name").toString();
    return e;
}

const char *kSelect =
    "SELECT e.*, emp.name AS employee_name, c.name AS category_name, "
    "sc.name AS subcategory_name, pm.name AS payment_method_name "
    "FROM Expenses e "
    "LEFT JOIN Employees emp ON emp.id = e.employee_id "
    "LEFT JOIN Categories c ON c.id = e.category_id "
    "LEFT JOIN SubCategories sc ON sc.id = e.subcategory_id "
    "LEFT JOIN PaymentMethods pm ON pm.id = e.payment_method_id ";

QVariant nullable(qint64 id) { return id > 0 ? QVariant(id) : QVariant(); }

} // namespace

QVector<Expense> ExpenseRepository::query(const TransactionFilter &f) const
{
    QVector<Expense> out;
    QVariantList binds;
    const QString sql = kSelect + whereClause(f, binds) + " ORDER BY e.date DESC, e.id DESC";
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const QVariant &b : binds) q.addBindValue(b);
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next()) out.append(fromRecord(q));
    return out;
}

std::optional<Expense> ExpenseRepository::byId(qint64 id) const
{
    QSqlQuery q(m_db);
    q.prepare(QString(kSelect) + "WHERE e.id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) return fromRecord(q);
    m_lastError = q.lastError().text();
    return std::nullopt;
}

qint64 ExpenseRepository::insert(const Expense &e)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Expenses(date, employee_id, description, category_id, "
              "subcategory_id, amount, payment_method_id, reference_no, remarks) "
              "VALUES(?,?,?,?,?,?,?,?,?)");
    q.addBindValue(e.date.toString(Qt::ISODate));
    q.addBindValue(nullable(e.employeeId));
    q.addBindValue(e.description);
    q.addBindValue(nullable(e.categoryId));
    q.addBindValue(nullable(e.subCategoryId));
    q.addBindValue(e.amountCents);
    q.addBindValue(nullable(e.paymentMethodId));
    q.addBindValue(e.referenceNo);
    q.addBindValue(e.remarks);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool ExpenseRepository::update(const Expense &e)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE Expenses SET date=?, employee_id=?, description=?, category_id=?, "
              "subcategory_id=?, amount=?, payment_method_id=?, reference_no=?, remarks=?, "
              "updated_at=datetime('now') WHERE id=?");
    q.addBindValue(e.date.toString(Qt::ISODate));
    q.addBindValue(nullable(e.employeeId));
    q.addBindValue(e.description);
    q.addBindValue(nullable(e.categoryId));
    q.addBindValue(nullable(e.subCategoryId));
    q.addBindValue(e.amountCents);
    q.addBindValue(nullable(e.paymentMethodId));
    q.addBindValue(e.referenceNo);
    q.addBindValue(e.remarks);
    q.addBindValue(e.id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool ExpenseRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Expenses WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool ExpenseRepository::restore(const Expense &e)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Expenses(id, date, employee_id, description, category_id, "
              "subcategory_id, amount, payment_method_id, reference_no, remarks) "
              "VALUES(?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(e.id);
    q.addBindValue(e.date.toString(Qt::ISODate));
    q.addBindValue(nullable(e.employeeId));
    q.addBindValue(e.description);
    q.addBindValue(nullable(e.categoryId));
    q.addBindValue(nullable(e.subCategoryId));
    q.addBindValue(e.amountCents);
    q.addBindValue(nullable(e.paymentMethodId));
    q.addBindValue(e.referenceNo);
    q.addBindValue(e.remarks);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

qint64 ExpenseRepository::totalCents(const TransactionFilter &f) const
{
    QVariantList binds;
    QSqlQuery q(m_db);
    q.prepare("SELECT COALESCE(SUM(e.amount),0) FROM Expenses e " + whereClause(f, binds));
    for (const QVariant &b : binds) q.addBindValue(b);
    if (!q.exec() || !q.next()) { m_lastError = q.lastError().text(); return 0; }
    return q.value(0).toLongLong();
}

} // namespace eb
