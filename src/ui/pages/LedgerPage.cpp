#include "LedgerPage.h"
#include "core/AppSettings.h"
#include "ui/AppContext.h"
#include "utils/Money.h"

#include <QDateEdit>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>

#include <algorithm>

namespace eb {

LedgerPage::LedgerPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *bar = new QHBoxLayout;
    bar->addWidget(new QLabel("From"));
    m_from = new QDateEdit(QDate(QDate::currentDate().year(), 1, 1));
    m_from->setCalendarPopup(true);
    bar->addWidget(m_from);
    bar->addWidget(new QLabel("To"));
    m_to = new QDateEdit(QDate::currentDate());
    m_to->setCalendarPopup(true);
    bar->addWidget(m_to);
    auto *apply = new QPushButton("Apply");
    bar->addWidget(apply);
    bar->addStretch();
    auto *print = new QPushButton("Print…");
    auto *csv = new QPushButton("Export CSV");
    bar->addWidget(print);
    bar->addWidget(csv);
    layout->addLayout(bar);

    m_table = new QTableWidget(0, 8);
    m_table->setHorizontalHeaderLabels({"Date", "Number", "Description", "Category",
                                        "Method", "Debit", "Credit", "Balance"});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->hide();
    layout->addWidget(m_table, 1);

    m_totals = new QLabel;
    m_totals->setObjectName("reportTotal");
    layout->addWidget(m_totals);

    connect(apply, &QPushButton::clicked, this, &LedgerPage::refresh);
    connect(print, &QPushButton::clicked, this, [this] {
        QPrinter printer(QPrinter::HighResolution);
        const QString preferred = AppSettings::instance().defaultPrinter();
        if (!preferred.isEmpty())
            printer.setPrinterName(preferred);
        QPrintDialog dlg(&printer, this);
        if (dlg.exec() != QDialog::Accepted)
            return;
        QTextDocument doc;
        doc.setHtml(ledgerHtml(buildLines()));
        doc.print(&printer);
    });
    connect(csv, &QPushButton::clicked, this, [this] {
        QString f = QFileDialog::getSaveFileName(this, "Export ledger", QString(),
                                                 "CSV (*.csv)");
        if (f.isEmpty())
            return;
        if (!f.endsWith(".csv", Qt::CaseInsensitive))
            f += ".csv";
        QFile out(f);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Export failed", out.errorString());
            return;
        }
        QTextStream ts(&out);
        ts << "Date,Number,Description,Category,Method,Debit,Credit,Balance\n";
        for (const Line &l : buildLines()) {
            const QString desc = QString(l.description).replace('"', "\"\"");
            ts << l.date.toString(Qt::ISODate) << ',' << l.number << ",\"" << desc
               << "\"," << l.category << ',' << l.method << ','
               << QString::number(l.debitCents / 100.0, 'f', 2) << ','
               << QString::number(l.creditCents / 100.0, 'f', 2) << ','
               << QString::number(l.balanceCents / 100.0, 'f', 2) << '\n';
        }
    });

    refresh();
}

QVector<LedgerPage::Line> LedgerPage::buildLines() const
{
    TransactionFilter f;
    f.from = m_from->date();
    f.to = m_to->date();

    QVector<Line> lines;
    for (const Expense &e : m_ctx.expenses.list(f)) {
        Line l;
        l.date = e.date;
        l.number = QString("EXP-%1").arg(e.id, 6, 10, QChar('0'));
        l.description = e.description;
        l.category = e.categoryName;
        l.method = e.paymentMethodName;
        l.debitCents = e.amountCents;
        lines.append(l);
    }
    for (const Income &i : m_ctx.income.list(f)) {
        Line l;
        l.date = i.date;
        l.number = QString("INC-%1").arg(i.id, 6, 10, QChar('0'));
        l.description = i.description.isEmpty() ? i.source : i.description;
        l.category = i.categoryName;
        l.method = i.paymentMethodName;
        l.creditCents = i.amountCents;
        lines.append(l);
    }
    std::sort(lines.begin(), lines.end(), [](const Line &a, const Line &b) {
        return a.date != b.date ? a.date < b.date : a.number < b.number;
    });

    qint64 balance = 0;
    for (Line &l : lines) {
        balance += l.creditCents - l.debitCents;
        l.balanceCents = balance;
    }
    return lines;
}

void LedgerPage::refresh()
{
    const QVector<Line> lines = buildLines();
    m_table->setRowCount(lines.size());
    qint64 debit = 0, credit = 0;
    for (int r = 0; r < lines.size(); ++r) {
        const Line &l = lines[r];
        debit += l.debitCents;
        credit += l.creditCents;
        const QStringList cells{
            l.date.toString(Qt::ISODate), l.number, l.description, l.category,
            l.method,
            l.debitCents ? money::format(l.debitCents) : QString(),
            l.creditCents ? money::format(l.creditCents) : QString(),
            money::format(l.balanceCents)};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            if (c >= 5)
                it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(r, c, it);
        }
    }
    m_totals->setText(QString("Debit: %1    Credit: %2    Net: %3")
                          .arg(money::formatWithSymbol(debit),
                               money::formatWithSymbol(credit),
                               money::formatWithSymbol(credit - debit)));
}

QString LedgerPage::ledgerHtml(const QVector<Line> &lines) const
{
    const auto &s = AppSettings::instance();
    QString body;
    qint64 debit = 0, credit = 0;
    int i = 0;
    for (const Line &l : lines) {
        debit += l.debitCents;
        credit += l.creditCents;
        body += QString(
            "<tr style='background:%1'><td>%2</td><td>%3</td><td>%4</td>"
            "<td>%5</td><td>%6</td><td align='right'>%7</td>"
            "<td align='right'>%8</td><td align='right'>%9</td></tr>")
            .arg(i++ % 2 ? "#f4f6f8" : "#ffffff",
                 l.date.toString("dd MMM yyyy"), l.number,
                 l.description.toHtmlEscaped(), l.category.toHtmlEscaped(),
                 l.method.toHtmlEscaped(),
                 l.debitCents ? money::format(l.debitCents) : QString(),
                 l.creditCents ? money::format(l.creditCents) : QString(),
                 money::format(l.balanceCents));
    }
    return QString(
        "<html><body style='font-family:Arial, sans-serif; font-size:9pt;'>"
        "<h2 style='margin-bottom:0'>%1</h2>"
        "<h3 style='margin-top:2px; color:#444'>General Ledger — %2 to %3</h3>"
        "<table width='100%' cellspacing='0' cellpadding='4' "
        "style='border:1px solid #999'>"
        "<tr style='background:#2b579a; color:white'>"
        "<th align='left'>Date</th><th align='left'>Number</th>"
        "<th align='left'>Description</th><th align='left'>Category</th>"
        "<th align='left'>Method</th><th align='right'>Debit (%4)</th>"
        "<th align='right'>Credit (%4)</th><th align='right'>Balance</th></tr>"
        "%5"
        "<tr><td colspan='5' align='right'><b>TOTALS</b></td>"
        "<td align='right'><b>%6</b></td><td align='right'><b>%7</b></td>"
        "<td align='right'><b>%8</b></td></tr>"
        "</table></body></html>")
        .arg(s.companyName().toHtmlEscaped(),
             m_from->date().toString("dd MMM yyyy"),
             m_to->date().toString("dd MMM yyyy"), s.currencySymbol(), body,
             money::format(debit), money::format(credit),
             money::format(credit - debit));
}

} // namespace eb
