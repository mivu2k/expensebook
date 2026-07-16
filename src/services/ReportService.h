#pragma once
#include "models/Models.h"
#include <QDate>
#include <QSqlDatabase>
#include <QVector>

namespace eb {

struct ReportRow {          // one printable ledger line
    QDate date;
    QString employee, category, subCategory, description, method, reference;
    qint64 amountCents = 0;
};

struct NameValue { QString name; qint64 cents = 0; };
struct MonthValue { QString month; qint64 expenseCents = 0; qint64 incomeCents = 0; };

/// Aggregation queries powering the Dashboard and Reports pages.
class ReportService {
public:
    explicit ReportService(QSqlDatabase db) : m_db(db) {}

    QVector<ReportRow> expenseReport(const TransactionFilter &f) const;
    QVector<NameValue> expensesByCategory(const QDate &from, const QDate &to) const;
    /// Last @p months months of expense and income totals, oldest first.
    QVector<MonthValue> monthlyTotals(int months = 12) const;
    /// Yearly expense totals, oldest first.
    QVector<NameValue> yearlyExpenseTotals() const;

private:
    QSqlDatabase m_db;
};

/// Standard report period presets.
struct Period {
    static std::pair<QDate, QDate> daily()   { auto d = QDate::currentDate(); return {d, d}; }
    static std::pair<QDate, QDate> weekly()  {
        auto d = QDate::currentDate();
        auto start = d.addDays(1 - d.dayOfWeek());
        return {start, start.addDays(6)};
    }
    static std::pair<QDate, QDate> monthly() {
        auto d = QDate::currentDate();
        return {{d.year(), d.month(), 1}, {d.year(), d.month(), d.daysInMonth()}};
    }
    static std::pair<QDate, QDate> quarterly() {
        auto d = QDate::currentDate();
        int qStart = ((d.month() - 1) / 3) * 3 + 1;
        QDate s(d.year(), qStart, 1);
        QDate e = s.addMonths(3).addDays(-1);
        return {s, e};
    }
    static std::pair<QDate, QDate> yearly()  {
        auto d = QDate::currentDate();
        return {{d.year(), 1, 1}, {d.year(), 12, 31}};
    }
};

} // namespace eb
