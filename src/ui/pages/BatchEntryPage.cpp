#include "BatchEntryPage.h"
#include "ui/AppContext.h"
#include "utils/Money.h"

#include <QComboBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace eb {

namespace {
enum Col { ColSno, ColDate, ColDesc, ColCategory, ColSubCat, ColAmount, ColAction };
}

BatchEntryPage::BatchEntryPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QDate::currentDate().toString(Qt::ISODate) +
                             "  Expense Entry Sheet");
    title->setObjectName("sheetTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto *bar = new QHBoxLayout;
    auto *add = new QPushButton("Add Row");
    auto *recalc = new QPushButton("Recalculate");
    auto *save = new QPushButton("Save All");
    auto *print = new QPushButton("Print");
    bar->addWidget(add); bar->addWidget(recalc);
    bar->addWidget(save); bar->addWidget(print);
    bar->addStretch();
    layout->addLayout(bar);

    m_grid = new QTableWidget(0, 7);
    m_grid->setHorizontalHeaderLabels({"S.no", "Date", "Description", "Category",
                                       "Sub Cat", "Amount", "Action"});
    m_grid->horizontalHeader()->setSectionResizeMode(ColDesc, QHeaderView::Stretch);
    m_grid->setColumnWidth(ColSno, 44);
    m_grid->setColumnWidth(ColDate, 120);
    m_grid->setColumnWidth(ColCategory, 160);
    m_grid->setColumnWidth(ColSubCat, 150);
    m_grid->setColumnWidth(ColAmount, 110);
    m_grid->verticalHeader()->hide();
    layout->addWidget(m_grid, 2);

    auto *bottom = new QHBoxLayout;
    auto *summaryBox = new QVBoxLayout;
    auto *sumTitle = new QLabel("Summary by Category");
    sumTitle->setObjectName("summaryTitle");
    summaryBox->addWidget(sumTitle);
    m_summary = new QTableWidget(0, 2);
    m_summary->setHorizontalHeaderLabels({"Category", "Amount"});
    m_summary->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_summary->verticalHeader()->hide();
    m_summary->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_summary->setMaximumWidth(420);
    m_summary->setMaximumHeight(180);
    summaryBox->addWidget(m_summary);
    bottom->addLayout(summaryBox);
    bottom->addStretch();
    m_total = new QLabel("Total: —");
    m_total->setObjectName("reportTotal");
    bottom->addWidget(m_total, 0, Qt::AlignBottom);
    layout->addLayout(bottom, 1);

    connect(add, &QPushButton::clicked, this, &BatchEntryPage::addRow);
    connect(recalc, &QPushButton::clicked, this, &BatchEntryPage::recalculate);
    connect(save, &QPushButton::clicked, this, &BatchEntryPage::saveAll);
    connect(print, &QPushButton::clicked, this, &BatchEntryPage::printSheet);

    for (int i = 0; i < 3; ++i)
        addRow();
    recalculate();
}

void BatchEntryPage::addRow()
{
    const int r = m_grid->rowCount();
    m_grid->insertRow(r);

    auto *sno = new QTableWidgetItem(QString::number(r + 1));
    sno->setFlags(Qt::ItemIsEnabled);
    m_grid->setItem(r, ColSno, sno);

    auto *date = new QDateEdit(QDate::currentDate());
    date->setCalendarPopup(true);
    m_grid->setCellWidget(r, ColDate, date);

    auto *desc = new QLineEdit;
    desc->setPlaceholderText("Description");
    m_grid->setCellWidget(r, ColDesc, desc);

    auto *cat = new QComboBox;
    cat->addItem("Select", 0);
    for (const Category &c : m_ctx.categories.categories("expense"))
        cat->addItem(c.name, c.id);
    m_grid->setCellWidget(r, ColCategory, cat);

    auto *sub = new QComboBox;
    sub->addItem("Select", 0);
    m_grid->setCellWidget(r, ColSubCat, sub);
    connect(cat, &QComboBox::currentIndexChanged, this, [this, cat, sub] {
        sub->clear();
        sub->addItem("Select", 0);
        const qint64 id = cat->currentData().toLongLong();
        if (id > 0)
            for (const SubCategory &s : m_ctx.categories.subCategories(id))
                sub->addItem(s.name, s.id);
        recalculate();
    });

    auto *amount = new QLineEdit;
    amount->setPlaceholderText("0.00");
    amount->setAlignment(Qt::AlignRight);
    m_grid->setCellWidget(r, ColAmount, amount);
    connect(amount, &QLineEdit::editingFinished, this, &BatchEntryPage::recalculate);

    auto *del = new QPushButton("Delete");
    del->setObjectName("dangerButton");
    m_grid->setCellWidget(r, ColAction, del);
    connect(del, &QPushButton::clicked, this, [this, del] {
        for (int row = 0; row < m_grid->rowCount(); ++row)
            if (m_grid->cellWidget(row, ColAction) == del) {
                m_grid->removeRow(row);
                break;
            }
        for (int row = 0; row < m_grid->rowCount(); ++row)
            m_grid->item(row, ColSno)->setText(QString::number(row + 1));
        recalculate();
    });
}

BatchEntryPage::RowData BatchEntryPage::rowData(int row) const
{
    RowData d;
    if (auto *date = qobject_cast<QDateEdit *>(m_grid->cellWidget(row, ColDate)))
        d.date = date->date();
    if (auto *desc = qobject_cast<QLineEdit *>(m_grid->cellWidget(row, ColDesc)))
        d.description = desc->text().trimmed();
    if (auto *cat = qobject_cast<QComboBox *>(m_grid->cellWidget(row, ColCategory))) {
        d.categoryId = cat->currentData().toLongLong();
        d.categoryName = d.categoryId ? cat->currentText() : QString();
    }
    if (auto *sub = qobject_cast<QComboBox *>(m_grid->cellWidget(row, ColSubCat)))
        d.subCategoryId = sub->currentData().toLongLong();
    if (auto *amount = qobject_cast<QLineEdit *>(m_grid->cellWidget(row, ColAmount)))
        money::parse(amount->text(), d.amountCents);
    return d;
}

void BatchEntryPage::recalculate()
{
    qint64 total = 0;
    QMap<QString, qint64> byCategory;
    for (int r = 0; r < m_grid->rowCount(); ++r) {
        const RowData d = rowData(r);
        if (d.amountCents <= 0)
            continue;
        total += d.amountCents;
        byCategory[d.categoryName.isEmpty() ? "(none)" : d.categoryName]
            += d.amountCents;
    }

    m_summary->setRowCount(byCategory.size());
    int r = 0;
    for (auto it = byCategory.constBegin(); it != byCategory.constEnd(); ++it, ++r) {
        m_summary->setItem(r, 0, new QTableWidgetItem(it.key()));
        auto *amt = new QTableWidgetItem(money::format(it.value()));
        amt->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_summary->setItem(r, 1, amt);
    }
    m_total->setText("Total: " + money::formatWithSymbol(total));
}

void BatchEntryPage::saveAll()
{
    recalculate();
    // Validate everything first so a sheet is saved all-or-nothing.
    QVector<Expense> pending;
    for (int r = 0; r < m_grid->rowCount(); ++r) {
        const RowData d = rowData(r);
        if (d.amountCents <= 0 && d.description.isEmpty())
            continue; // untouched row
        if (d.amountCents <= 0 || d.categoryId <= 0 || d.description.isEmpty()) {
            QMessageBox::warning(this, "Validation",
                QString("Row %1 needs a description, category and amount.").arg(r + 1));
            return;
        }
        Expense e;
        e.date = d.date;
        e.description = d.description;
        e.categoryId = d.categoryId;
        e.subCategoryId = d.subCategoryId;
        e.amountCents = d.amountCents;
        pending.append(e);
    }
    if (pending.isEmpty()) {
        QMessageBox::information(this, "Batch entry", "Nothing to save yet.");
        return;
    }
    for (const Expense &e : pending) {
        if (!m_ctx.expenses.save(e)) {
            QMessageBox::critical(this, "Save failed", m_ctx.expenses.errorMessage());
            return;
        }
    }
    QMessageBox::information(this, "Batch entry",
        QString("%1 expense(s) saved.").arg(pending.size()));
    m_grid->setRowCount(0);
    for (int i = 0; i < 3; ++i)
        addRow();
    recalculate();
}

QVector<ReportRow> BatchEntryPage::asReportRows() const
{
    QVector<ReportRow> rows;
    for (int r = 0; r < m_grid->rowCount(); ++r) {
        const RowData d = rowData(r);
        if (d.amountCents <= 0)
            continue;
        ReportRow row;
        row.date = d.date;
        row.description = d.description;
        row.category = d.categoryName;
        row.amountCents = d.amountCents;
        rows.append(row);
    }
    return rows;
}

void BatchEntryPage::printSheet()
{
    m_ctx.reports.print(this,
        QDate::currentDate().toString(Qt::ISODate) + " Expense Entry Sheet",
        asReportRows());
}

void BatchEntryPage::refresh()
{
    // Refresh the category combos of every row in place.
    for (int r = 0; r < m_grid->rowCount(); ++r) {
        auto *cat = qobject_cast<QComboBox *>(m_grid->cellWidget(r, ColCategory));
        if (!cat)
            continue;
        const qint64 keep = cat->currentData().toLongLong();
        cat->blockSignals(true);
        cat->clear();
        cat->addItem("Select", 0);
        for (const Category &c : m_ctx.categories.categories("expense"))
            cat->addItem(c.name, c.id);
        cat->setCurrentIndex(qMax(0, cat->findData(keep)));
        cat->blockSignals(false);
    }
}

} // namespace eb
