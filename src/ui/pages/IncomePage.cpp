#include "IncomePage.h"
#include "ui/AppContext.h"
#include "utils/Money.h"

#include <QComboBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>

namespace eb {

IncomePage::IncomePage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *root = new QHBoxLayout(this);

    auto *formBox = new QGroupBox("Income Entry");
    formBox->setMaximumWidth(420);
    auto *form = new QFormLayout(formBox);

    m_date = new QDateEdit(QDate::currentDate());
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat("dd MMM yyyy");
    form->addRow("Date*", m_date);
    form->addRow("Source*", m_source = new QLineEdit);
    form->addRow("Description", m_description = new QLineEdit);
    form->addRow("Category", m_category = new QComboBox);
    form->addRow("Amount*", m_amount = new QLineEdit);
    m_amount->setPlaceholderText("0.00");
    form->addRow("Payment Method", m_method = new QComboBox);
    form->addRow("Reference #", m_reference = new QLineEdit);
    m_remarks = new QPlainTextEdit;
    m_remarks->setMaximumHeight(60);
    form->addRow("Remarks", m_remarks);

    auto *buttons = new QHBoxLayout;
    auto *save = new QPushButton("Save");
    auto *saveNew = new QPushButton("Save && New");
    auto *clear = new QPushButton("Clear");
    auto *del = new QPushButton("Delete");
    for (auto *b : {save, saveNew, clear, del})
        buttons->addWidget(b);
    form->addRow(buttons);
    root->addWidget(formBox);

    m_table = new QTableWidget;
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"Date", "Source", "Description", "Category",
                                        "Method", "Ref #", "Amount"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    connect(save, &QPushButton::clicked, this, [this] { onSave(false); });
    connect(saveNew, &QPushButton::clicked, this, [this] { onSave(true); });
    connect(clear, &QPushButton::clicked, this, &IncomePage::onClear);
    connect(del, &QPushButton::clicked, this, &IncomePage::onDelete);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &IncomePage::onRowSelected);

    refresh();
}

void IncomePage::refresh()
{
    const qint64 cat = m_category->currentData().toLongLong();
    const qint64 pm = m_method->currentData().toLongLong();

    m_category->clear();
    m_category->addItem("(none)", 0);
    for (const Category &c : m_ctx.categories.categories("income"))
        m_category->addItem(c.name, c.id);
    m_method->clear();
    m_method->addItem("(none)", 0);
    for (const PaymentMethod &p : m_ctx.paymentMethods.all())
        m_method->addItem(p.name, p.id);

    m_category->setCurrentIndex(qMax(0, m_category->findData(cat)));
    m_method->setCurrentIndex(qMax(0, m_method->findData(pm)));
    reloadTable();
}

void IncomePage::setSearchText(const QString &text)
{
    m_search = text;
    reloadTable();
}

void IncomePage::reloadTable()
{
    TransactionFilter f;
    f.text = m_search;
    const QVector<Income> rows = m_ctx.income.list(f);

    m_table->setSortingEnabled(false);
    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        const Income &i = rows[r];
        const QStringList cells{i.date.toString(Qt::ISODate), i.source, i.description,
                                i.categoryName, i.paymentMethodName, i.referenceNo,
                                money::format(i.amountCents)};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            it->setData(Qt::UserRole, i.id);
            if (c == 6)
                it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(r, c, it);
        }
    }
    m_table->setSortingEnabled(true);
}

Income IncomePage::collect() const
{
    Income i;
    i.id = m_editingId;
    i.date = m_date->date();
    i.source = m_source->text().trimmed();
    i.description = m_description->text().trimmed();
    i.categoryId = m_category->currentData().toLongLong();
    money::parse(m_amount->text(), i.amountCents);
    i.paymentMethodId = m_method->currentData().toLongLong();
    i.referenceNo = m_reference->text().trimmed();
    i.remarks = m_remarks->toPlainText().trimmed();
    return i;
}

void IncomePage::onSave(bool andNew)
{
    qint64 amount = 0;
    if (!money::parse(m_amount->text(), amount)) {
        QMessageBox::warning(this, "Validation", "Enter a valid amount.");
        return;
    }
    const qint64 id = m_ctx.income.save(collect());
    if (!id) {
        QMessageBox::warning(this, "Save failed", m_ctx.income.errorMessage());
        return;
    }
    reloadTable();
    if (andNew)
        onClear();
    else
        m_editingId = id;
}

void IncomePage::onClear()
{
    m_editingId = 0;
    m_date->setDate(QDate::currentDate());
    m_source->clear();
    m_description->clear();
    m_category->setCurrentIndex(0);
    m_amount->clear();
    m_method->setCurrentIndex(0);
    m_reference->clear();
    m_remarks->clear();
    m_source->setFocus();
}

void IncomePage::onDelete()
{
    if (!m_editingId) {
        QMessageBox::information(this, "Delete", "Select an income record first.");
        return;
    }
    if (QMessageBox::question(this, "Delete income",
            "Delete this record? You can undo with Ctrl+Z.") != QMessageBox::Yes)
        return;
    if (!m_ctx.income.remove(m_editingId)) {
        QMessageBox::warning(this, "Delete failed", m_ctx.income.errorMessage());
        return;
    }
    onClear();
    reloadTable();
}

void IncomePage::onRowSelected()
{
    const auto items = m_table->selectedItems();
    if (items.isEmpty())
        return;
    const qint64 id = items.first()->data(Qt::UserRole).toLongLong();
    auto rec = m_ctx.income.get(id);
    if (!rec)
        return;
    m_editingId = rec->id;
    m_date->setDate(rec->date);
    m_source->setText(rec->source);
    m_description->setText(rec->description);
    m_category->setCurrentIndex(qMax(0, m_category->findData(rec->categoryId)));
    m_amount->setText(money::format(rec->amountCents).remove(','));
    m_method->setCurrentIndex(qMax(0, m_method->findData(rec->paymentMethodId)));
    m_reference->setText(rec->referenceNo);
    m_remarks->setPlainText(rec->remarks);
}

} // namespace eb
