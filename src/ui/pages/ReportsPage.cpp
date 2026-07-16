#include "ReportsPage.h"
#include "ui/AppContext.h"
#include "utils/Money.h"

#include <QComboBox>
#include <QDateEdit>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>

namespace eb {

ReportsPage::ReportsPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *filterBox = new QGroupBox("Report Filters");
    auto *grid = new QGridLayout(filterBox);

    m_period = new QComboBox;
    m_period->addItems({"Daily", "Weekly", "Monthly", "Quarterly", "Yearly",
                        "Custom Range"});
    m_period->setCurrentIndex(2);
    grid->addWidget(new QLabel("Period"), 0, 0);
    grid->addWidget(m_period, 0, 1);

    m_from = new QDateEdit; m_from->setCalendarPopup(true);
    m_to = new QDateEdit;   m_to->setCalendarPopup(true);
    grid->addWidget(new QLabel("From"), 0, 2);
    grid->addWidget(m_from, 0, 3);
    grid->addWidget(new QLabel("To"), 0, 4);
    grid->addWidget(m_to, 0, 5);

    grid->addWidget(new QLabel("Employee"), 1, 0);
    grid->addWidget(m_employee = new QComboBox, 1, 1);
    grid->addWidget(new QLabel("Category"), 1, 2);
    grid->addWidget(m_category = new QComboBox, 1, 3);
    grid->addWidget(new QLabel("Sub Category"), 1, 4);
    grid->addWidget(m_subCategory = new QComboBox, 1, 5);

    grid->addWidget(new QLabel("Payment"), 2, 0);
    grid->addWidget(m_method = new QComboBox, 2, 1);
    grid->addWidget(new QLabel("Amount ≥"), 2, 2);
    grid->addWidget(m_minAmount = new QLineEdit, 2, 3);
    grid->addWidget(new QLabel("Amount ≤"), 2, 4);
    grid->addWidget(m_maxAmount = new QLineEdit, 2, 5);

    grid->addWidget(new QLabel("Description"), 3, 0);
    grid->addWidget(m_text = new QLineEdit, 3, 1, 1, 3);
    auto *run = new QPushButton("Run Report");
    grid->addWidget(run, 3, 5);
    layout->addWidget(filterBox);

    m_table = new QTableWidget;
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"Date", "Employee", "Category", "Sub Category",
                                        "Description", "Method", "Ref #", "Amount"});
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    layout->addWidget(m_table, 1);

    auto *bottom = new QHBoxLayout;
    m_total = new QLabel("Total: —");
    m_total->setObjectName("reportTotal");
    bottom->addWidget(m_total);
    bottom->addStretch();
    auto *csv = new QPushButton("Export CSV");
    auto *xlsx = new QPushButton("Export Excel");
    auto *pdf = new QPushButton("Export PDF");
    auto *print = new QPushButton("Print…");
    for (auto *b : {csv, xlsx, pdf, print})
        bottom->addWidget(b);
    layout->addLayout(bottom);

    connect(m_period, &QComboBox::currentIndexChanged,
            this, &ReportsPage::applyPeriodPreset);
    connect(m_category, &QComboBox::currentIndexChanged, this, [this] {
        m_subCategory->clear();
        m_subCategory->addItem("(any)", 0);
        const qint64 id = m_category->currentData().toLongLong();
        if (id > 0)
            for (const SubCategory &s : m_ctx.categories.subCategories(id))
                m_subCategory->addItem(s.name, s.id);
    });
    connect(run, &QPushButton::clicked, this, &ReportsPage::runReport);

    const auto pickFile = [this](const QString &filter, const QString &ext) {
        QString f = QFileDialog::getSaveFileName(this, "Export report", QString(), filter);
        if (!f.isEmpty() && !f.endsWith(ext, Qt::CaseInsensitive))
            f += ext;
        return f;
    };
    connect(csv, &QPushButton::clicked, this, [this, pickFile] {
        const QString f = pickFile("CSV (*.csv)", ".csv");
        if (f.isEmpty()) return;
        if (!m_ctx.reports.exportCsv(f, m_rows))
            QMessageBox::warning(this, "Export failed", m_ctx.reports.errorMessage());
    });
    connect(xlsx, &QPushButton::clicked, this, [this, pickFile] {
        const QString f = pickFile("Excel (*.xlsx)", ".xlsx");
        if (f.isEmpty()) return;
        if (!m_ctx.reports.exportXlsx(f, m_rows))
            QMessageBox::warning(this, "Export failed", m_ctx.reports.errorMessage());
    });
    connect(pdf, &QPushButton::clicked, this, [this, pickFile] {
        const QString f = pickFile("PDF (*.pdf)", ".pdf");
        if (f.isEmpty()) return;
        if (!m_ctx.reports.exportPdf(f, reportTitle(), m_rows))
            QMessageBox::warning(this, "Export failed", m_ctx.reports.errorMessage());
    });
    connect(print, &QPushButton::clicked, this, [this] {
        m_ctx.reports.print(this, reportTitle(), m_rows);
    });

    refresh();
    applyPeriodPreset(m_period->currentIndex());
    runReport();
}

void ReportsPage::refresh()
{
    m_employee->clear();
    m_employee->addItem("(any)", 0);
    for (const Employee &e : m_ctx.employees.list())
        m_employee->addItem(e.name, e.id);

    m_category->clear();
    m_category->addItem("(any)", 0);
    for (const Category &c : m_ctx.categories.categories())
        m_category->addItem(c.name, c.id);

    m_method->clear();
    m_method->addItem("(any)", 0);
    for (const PaymentMethod &p : m_ctx.paymentMethods.all())
        m_method->addItem(p.name, p.id);
}

void ReportsPage::applyPeriodPreset(int index)
{
    std::pair<QDate, QDate> range;
    switch (index) {
    case 0: range = Period::daily(); break;
    case 1: range = Period::weekly(); break;
    case 2: range = Period::monthly(); break;
    case 3: range = Period::quarterly(); break;
    case 4: range = Period::yearly(); break;
    default: return; // Custom: leave the dates as the user set them
    }
    m_from->setDate(range.first);
    m_to->setDate(range.second);
}

QString ReportsPage::reportTitle() const
{
    return QString("Expense Report — %1 (%2 to %3)")
        .arg(m_period->currentText(),
             m_from->date().toString("dd MMM yyyy"),
             m_to->date().toString("dd MMM yyyy"));
}

void ReportsPage::runReport()
{
    TransactionFilter f;
    f.from = m_from->date();
    f.to = m_to->date();
    f.employeeId = m_employee->currentData().toLongLong();
    f.categoryId = m_category->currentData().toLongLong();
    f.subCategoryId = m_subCategory->currentData().toLongLong();
    f.paymentMethodId = m_method->currentData().toLongLong();
    f.text = m_text->text().trimmed();
    qint64 cents = 0;
    if (money::parse(m_minAmount->text(), cents) && !m_minAmount->text().isEmpty())
        f.minCents = cents;
    if (money::parse(m_maxAmount->text(), cents) && !m_maxAmount->text().isEmpty())
        f.maxCents = cents;

    m_rows = m_ctx.reports.run(f);

    m_table->setSortingEnabled(false);
    m_table->setRowCount(m_rows.size());
    qint64 total = 0;
    for (int r = 0; r < m_rows.size(); ++r) {
        const ReportRow &row = m_rows[r];
        total += row.amountCents;
        const QStringList cells{row.date.toString(Qt::ISODate), row.employee,
                                row.category, row.subCategory, row.description,
                                row.method, row.reference,
                                money::format(row.amountCents)};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            if (c == 7)
                it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(r, c, it);
        }
    }
    m_table->setSortingEnabled(true);
    m_total->setText(QString("Total: %1  (%2 records)")
                         .arg(money::formatWithSymbol(total))
                         .arg(m_rows.size()));
}

} // namespace eb
