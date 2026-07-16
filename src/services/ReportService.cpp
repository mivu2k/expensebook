#include "ReportService.h"
#include "repositories/ExpenseRepository.h"

#include <QSqlQuery>

namespace eb {

QVector<ReportRow> ReportService::expenseReport(const TransactionFilter &f) const
{
    QVector<ReportRow> out;
    ExpenseRepository repo(m_db);
    for (const Expense &e : repo.query(f)) {
        out.append({e.date, e.employeeName, e.categoryName, e.subCategoryName,
                    e.description, e.paymentMethodName, e.referenceNo, e.amountCents});
    }
    return out;
}

QVector<NameValue> ReportService::expensesByCategory(const QDate &from, const QDate &to) const
{
    QVector<NameValue> out;
    QSqlQuery q(m_db);
    q.prepare("SELECT COALESCE(c.name,'(Uncategorized)'), SUM(e.amount) FROM Expenses e "
              "LEFT JOIN Categories c ON c.id = e.category_id "
              "WHERE e.date BETWEEN ? AND ? GROUP BY c.name ORDER BY 2 DESC");
    q.addBindValue(from.toString(Qt::ISODate));
    q.addBindValue(to.toString(Qt::ISODate));
    if (q.exec())
        while (q.next())
            out.append({q.value(0).toString(), q.value(1).toLongLong()});
    return out;
}

QVector<MonthValue> ReportService::monthlyTotals(int months) const
{
    // Collect both series keyed by "yyyy-MM".
    QMap<QString, MonthValue> map;
    const QString startMonth =
        QDate::currentDate().addMonths(-(months - 1)).toString("yyyy-MM");

    QSqlQuery qe(m_db);
    qe.prepare("SELECT substr(date,1,7) m, SUM(amount) FROM Expenses "
               "WHERE substr(date,1,7) >= ? GROUP BY m");
    qe.addBindValue(startMonth);
    if (qe.exec())
        while (qe.next()) {
            auto &mv = map[qe.value(0).toString()];
            mv.month = qe.value(0).toString();
            mv.expenseCents = qe.value(1).toLongLong();
        }

    QSqlQuery qi(m_db);
    qi.prepare("SELECT substr(date,1,7) m, SUM(amount) FROM Income "
               "WHERE substr(date,1,7) >= ? GROUP BY m");
    qi.addBindValue(startMonth);
    if (qi.exec())
        while (qi.next()) {
            auto &mv = map[qi.value(0).toString()];
            mv.month = qi.value(0).toString();
            mv.incomeCents = qi.value(1).toLongLong();
        }

    // Emit a contiguous month range so charts have no gaps.
    QVector<MonthValue> out;
    QDate d(QDate::currentDate().addMonths(-(months - 1)));
    d = QDate(d.year(), d.month(), 1);
    for (int i = 0; i < months; ++i) {
        const QString key = d.toString("yyyy-MM");
        MonthValue mv = map.value(key);
        mv.month = key;
        out.append(mv);
        d = d.addMonths(1);
    }
    return out;
}

QVector<NameValue> ReportService::yearlyExpenseTotals() const
{
    QVector<NameValue> out;
    QSqlQuery q(m_db);
    if (q.exec("SELECT substr(date,1,4) y, SUM(amount) FROM Expenses GROUP BY y ORDER BY y"))
        while (q.next())
            out.append({q.value(0).toString(), q.value(1).toLongLong()});
    return out;
}

} // namespace eb
