#include "ExpensePage.h"
#include "ui/AppContext.h"
#include "ui/widgets/AttachmentListWidget.h"
#include "utils/Money.h"

#include <QComboBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>

namespace eb {

ExpensePage::ExpensePage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *root = new QHBoxLayout(this);

    // ----- Left: entry form -----
    auto *formBox = new QGroupBox("Expense Entry");
    formBox->setMaximumWidth(420);
    auto *form = new QFormLayout(formBox);

    m_date = new QDateEdit(QDate::currentDate());
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat("dd MMM yyyy");
    form->addRow("Date*", m_date);

    form->addRow("Employee", m_employee = new QComboBox);
    form->addRow("Description*", m_description = new QLineEdit);
    form->addRow("Category*", m_category = new QComboBox);
    form->addRow("Sub Category", m_subCategory = new QComboBox);
    form->addRow("Amount*", m_amount = new QLineEdit);
    m_amount->setPlaceholderText("0.00");
    form->addRow("Payment Method", m_method = new QComboBox);
    form->addRow("Reference #", m_reference = new QLineEdit);
    m_remarks = new QPlainTextEdit;
    m_remarks->setMaximumHeight(60);
    form->addRow("Remarks", m_remarks);
    form->addRow("Attachments", m_attachments = new AttachmentListWidget);

    auto *buttons = new QHBoxLayout;
    auto *save = new QPushButton("Save");
    auto *saveNew = new QPushButton("Save && New");
    auto *clear = new QPushButton("Clear");
    auto *del = new QPushButton("Delete");
    auto *dup = new QPushButton("Duplicate");
    save->setDefault(true);
    for (auto *b : {save, saveNew, clear, del, dup})
        buttons->addWidget(b);
    form->addRow(buttons);
    root->addWidget(formBox);

    // ----- Right: expense table -----
    m_table = new QTableWidget;
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"Date", "Employee", "Description", "Category",
                                        "Sub Category", "Method", "Ref #", "Amount"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    // Category → sub-category cascade
    connect(m_category, &QComboBox::currentIndexChanged, this, [this] {
        m_subCategory->clear();
        m_subCategory->addItem("(none)", 0);
        const qint64 catId = m_category->currentData().toLongLong();
        if (catId > 0)
            for (const SubCategory &s : m_ctx.categories.subCategories(catId))
                m_subCategory->addItem(s.name, s.id);
    });

    connect(save, &QPushButton::clicked, this, [this] { onSave(false); });
    connect(saveNew, &QPushButton::clicked, this, [this] { onSave(true); });
    connect(clear, &QPushButton::clicked, this, &ExpensePage::onClear);
    connect(del, &QPushButton::clicked, this, &ExpensePage::onDelete);
    connect(dup, &QPushButton::clicked, this, &ExpensePage::onDuplicate);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &ExpensePage::onRowSelected);
    connect(m_attachments, &AttachmentListWidget::previewRequested, this,
            [this](qint64 id, const QString &name) {
                if (id <= 0) return;
                const QByteArray data = m_ctx.expenses.attachmentData(id);
                QPixmap px;
                if (px.loadFromData(data)) {
                    auto *dlg = new QLabel(nullptr, Qt::Window);
                    dlg->setAttribute(Qt::WA_DeleteOnClose);
                    dlg->setWindowTitle(name);
                    dlg->setPixmap(px.scaled(800, 600, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
                    dlg->show();
                } else {
                    QMessageBox::information(this, "Preview",
                        "Preview is available for image attachments only.");
                }
            });

    refresh();
}

void ExpensePage::reloadLookups()
{
    const auto keep = [](QComboBox *cb) { return cb->currentData().toLongLong(); };
    const auto restore = [](QComboBox *cb, qint64 id) {
        const int ix = cb->findData(id);
        cb->setCurrentIndex(ix >= 0 ? ix : 0);
    };

    const qint64 emp = keep(m_employee), cat = keep(m_category), pm = keep(m_method);

    m_employee->clear();
    m_employee->addItem("(none)", 0);
    for (const Employee &e : m_ctx.employees.list(true))
        m_employee->addItem(e.name, e.id);

    m_category->clear();
    m_category->addItem("(select)", 0);
    for (const Category &c : m_ctx.categories.categories("expense"))
        m_category->addItem(c.name, c.id);

    m_method->clear();
    m_method->addItem("(none)", 0);
    for (const PaymentMethod &p : m_ctx.paymentMethods.all())
        m_method->addItem(p.name, p.id);

    restore(m_employee, emp);
    restore(m_category, cat);
    restore(m_method, pm);
}

void ExpensePage::reloadTable()
{
    TransactionFilter f;
    f.text = m_search;
    const QVector<Expense> rows = m_ctx.expenses.list(f);

    m_table->setSortingEnabled(false);
    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        const Expense &e = rows[r];
        const QStringList cells{e.date.toString(Qt::ISODate), e.employeeName,
                                e.description, e.categoryName, e.subCategoryName,
                                e.paymentMethodName, e.referenceNo,
                                money::format(e.amountCents)};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            it->setData(Qt::UserRole, e.id);
            if (c == 7)
                it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(r, c, it);
        }
    }
    m_table->setSortingEnabled(true);
}

void ExpensePage::refresh()
{
    reloadLookups();
    reloadTable();
}

void ExpensePage::setSearchText(const QString &text)
{
    m_search = text;
    reloadTable();
}

Expense ExpensePage::collect() const
{
    Expense e;
    e.id = m_editingId;
    e.date = m_date->date();
    e.employeeId = m_employee->currentData().toLongLong();
    e.description = m_description->text().trimmed();
    e.categoryId = m_category->currentData().toLongLong();
    e.subCategoryId = m_subCategory->currentData().toLongLong();
    money::parse(m_amount->text(), e.amountCents);
    e.paymentMethodId = m_method->currentData().toLongLong();
    e.referenceNo = m_reference->text().trimmed();
    e.remarks = m_remarks->toPlainText().trimmed();
    return e;
}

void ExpensePage::loadIntoForm(const Expense &e)
{
    m_editingId = e.id;
    m_date->setDate(e.date);
    m_employee->setCurrentIndex(qMax(0, m_employee->findData(e.employeeId)));
    m_description->setText(e.description);
    m_category->setCurrentIndex(qMax(0, m_category->findData(e.categoryId)));
    m_subCategory->setCurrentIndex(qMax(0, m_subCategory->findData(e.subCategoryId)));
    m_amount->setText(money::format(e.amountCents).remove(','));
    m_method->setCurrentIndex(qMax(0, m_method->findData(e.paymentMethodId)));
    m_reference->setText(e.referenceNo);
    m_remarks->setPlainText(e.remarks);
    m_attachments->setExisting(m_ctx.expenses.attachments(e.id));
}

void ExpensePage::persistAttachments(qint64 expenseId)
{
    for (const qint64 removed : m_attachments->removedIds())
        m_ctx.expenses.removeAttachment(removed);
    for (Attachment a : m_attachments->stagedNew()) {
        if (a.fileName.isEmpty())
            continue; // removed before saving
        a.expenseId = expenseId;
        m_ctx.expenses.addAttachment(a);
    }
}

void ExpensePage::onSave(bool andNew)
{
    qint64 amount = 0;
    if (!money::parse(m_amount->text(), amount)) {
        QMessageBox::warning(this, "Validation", "Enter a valid amount.");
        return;
    }
    const qint64 id = m_ctx.expenses.save(collect());
    if (!id) {
        QMessageBox::warning(this, "Save failed", m_ctx.expenses.errorMessage());
        return;
    }
    persistAttachments(id);
    reloadTable();
    if (andNew)
        onClear();
    else
        m_editingId = id;
}

void ExpensePage::saveCurrent() { onSave(false); }

void ExpensePage::onClear()
{
    m_editingId = 0;
    m_date->setDate(QDate::currentDate());
    m_employee->setCurrentIndex(0);
    m_description->clear();
    m_category->setCurrentIndex(0);
    m_amount->clear();
    m_method->setCurrentIndex(0);
    m_reference->clear();
    m_remarks->clear();
    m_attachments->clearAll();
    m_description->setFocus();
}

void ExpensePage::onDelete()
{
    if (!m_editingId) {
        QMessageBox::information(this, "Delete", "Select an expense first.");
        return;
    }
    if (QMessageBox::question(this, "Delete expense",
            "Delete this expense? You can undo with Ctrl+Z.") != QMessageBox::Yes)
        return;
    if (!m_ctx.expenses.remove(m_editingId)) {
        QMessageBox::warning(this, "Delete failed", m_ctx.expenses.errorMessage());
        return;
    }
    onClear();
    reloadTable();
}

void ExpensePage::onDuplicate()
{
    if (!m_editingId) {
        QMessageBox::information(this, "Duplicate", "Select an expense first.");
        return;
    }
    const qint64 id = m_ctx.expenses.duplicate(m_editingId);
    if (!id) {
        QMessageBox::warning(this, "Duplicate failed", m_ctx.expenses.errorMessage());
        return;
    }
    reloadTable();
}

void ExpensePage::onRowSelected()
{
    const auto items = m_table->selectedItems();
    if (items.isEmpty())
        return;
    const qint64 id = items.first()->data(Qt::UserRole).toLongLong();
    if (auto e = m_ctx.expenses.get(id))
        loadIntoForm(*e);
}

} // namespace eb
