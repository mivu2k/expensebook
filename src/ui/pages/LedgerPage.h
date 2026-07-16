#pragma once
#include <QDate>
#include <QWidget>

class QDateEdit;
class QLabel;
class QTableWidget;

namespace eb {

struct AppContext;

/// Classic accounting ledger: expenses and income merged in date order
/// with debit / credit columns and a running balance, printable and
/// exportable to CSV.
class LedgerPage : public QWidget {
    Q_OBJECT
public:
    explicit LedgerPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    struct Line {
        QDate date;
        QString number, description, category, method;
        qint64 debitCents = 0;   // expense
        qint64 creditCents = 0;  // income
        qint64 balanceCents = 0; // running credit - debit
    };
    QVector<Line> buildLines() const;
    QString ledgerHtml(const QVector<Line> &lines) const;

    AppContext &m_ctx;
    QDateEdit *m_from, *m_to;
    QTableWidget *m_table;
    QLabel *m_totals;
};

} // namespace eb
