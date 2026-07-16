#include "IncomeRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

namespace {

QString whereClause(const TransactionFilter &f, QVariantList &binds)
{
    QStringList w;
    if (f.from.isValid()) { w << "i.date >= ?"; binds << f.from.toString(Qt::ISODate); }
    if (f.to.isValid())   { w << "i.date <= ?"; binds << f.to.toString(Qt::ISODate); }
    if (f.categoryId)      { w << "i.category_id = ?";       binds << f.categoryId; }
    if (f.paymentMethodId) { w << "i.payment_method_id = ?"; binds << f.paymentMethodId; }
    if (f.minCents >= 0)   { w << "i.amount >= ?";           binds << f.minCents; }
    if (f.maxCents >= 0)   { w << "i.amount <= ?";           binds << f.maxCents; }
    if (!f.text.isEmpty()) {
        w << "(i.description LIKE ? OR i.source LIKE ? OR i.reference_no LIKE ?)";
        const QString like = "%" + f.text + "%";
        binds << like << like << like;
    }
    return w.isEmpty() ? "" : "WHERE " + w.join(" AND ");
}

Income fromRecord(const QSqlQuery &q)
{
    Income i;
    i.id = q.value("id").toLongLong();
    i.date = QDate::fromString(q.value("date").toString(), Qt::ISODate);
    i.source = q.value("source").toString();
    i.description = q.value("description").toString();
    i.categoryId = q.value("category_id").toLongLong();
    i.amountCents = q.value("amount").toLongLong();
    i.paymentMethodId = q.value("payment_method_id").toLongLong();
    i.referenceNo = q.value("reference_no").toString();
    i.remarks = q.value("remarks").toString();
    i.categoryName = q.value("category_name").toString();
    i.paymentMethodName = q.value("payment_method_name").toString();
    return i;
}

const char *kSelect =
    "SELECT i.*, c.name AS category_name, pm.name AS payment_method_name "
    "FROM Income i "
    "LEFT JOIN Categories c ON c.id = i.category_id "
    "LEFT JOIN PaymentMethods pm ON pm.id = i.payment_method_id ";

QVariant nullable(qint64 id) { return id > 0 ? QVariant(id) : QVariant(); }

} // namespace

QVector<Income> IncomeRepository::query(const TransactionFilter &f) const
{
    QVector<Income> out;
    QVariantList binds;
    QSqlQuery q(m_db);
    q.prepare(kSelect + whereClause(f, binds) + " ORDER BY i.date DESC, i.id DESC");
    for (const QVariant &b : binds) q.addBindValue(b);
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next()) out.append(fromRecord(q));
    return out;
}

std::optional<Income> IncomeRepository::byId(qint64 id) const
{
    QSqlQuery q(m_db);
    q.prepare(QString(kSelect) + "WHERE i.id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) return fromRecord(q);
    m_lastError = q.lastError().text();
    return std::nullopt;
}

qint64 IncomeRepository::insert(const Income &i)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Income(date, source, description, category_id, amount, "
              "payment_method_id, reference_no, remarks) VALUES(?,?,?,?,?,?,?,?)");
    q.addBindValue(i.date.toString(Qt::ISODate));
    q.addBindValue(i.source);
    q.addBindValue(i.description);
    q.addBindValue(nullable(i.categoryId));
    q.addBindValue(i.amountCents);
    q.addBindValue(nullable(i.paymentMethodId));
    q.addBindValue(i.referenceNo);
    q.addBindValue(i.remarks);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool IncomeRepository::update(const Income &i)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE Income SET date=?, source=?, description=?, category_id=?, amount=?, "
              "payment_method_id=?, reference_no=?, remarks=? WHERE id=?");
    q.addBindValue(i.date.toString(Qt::ISODate));
    q.addBindValue(i.source);
    q.addBindValue(i.description);
    q.addBindValue(nullable(i.categoryId));
    q.addBindValue(i.amountCents);
    q.addBindValue(nullable(i.paymentMethodId));
    q.addBindValue(i.referenceNo);
    q.addBindValue(i.remarks);
    q.addBindValue(i.id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool IncomeRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Income WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

bool IncomeRepository::restore(const Income &i)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Income(id, date, source, description, category_id, amount, "
              "payment_method_id, reference_no, remarks) VALUES(?,?,?,?,?,?,?,?,?)");
    q.addBindValue(i.id);
    q.addBindValue(i.date.toString(Qt::ISODate));
    q.addBindValue(i.source);
    q.addBindValue(i.description);
    q.addBindValue(nullable(i.categoryId));
    q.addBindValue(i.amountCents);
    q.addBindValue(nullable(i.paymentMethodId));
    q.addBindValue(i.referenceNo);
    q.addBindValue(i.remarks);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

qint64 IncomeRepository::totalCents(const TransactionFilter &f) const
{
    QVariantList binds;
    QSqlQuery q(m_db);
    q.prepare("SELECT COALESCE(SUM(i.amount),0) FROM Income i " + whereClause(f, binds));
    for (const QVariant &b : binds) q.addBindValue(b);
    if (!q.exec() || !q.next()) { m_lastError = q.lastError().text(); return 0; }
    return q.value(0).toLongLong();
}

} // namespace eb
