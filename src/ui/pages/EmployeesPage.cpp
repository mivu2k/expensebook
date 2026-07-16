#include "EmployeesPage.h"
#include "ui/AppContext.h"
#include "ui/dialogs/EmployeeDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace eb {

EmployeesPage::EmployeesPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *bar = new QHBoxLayout;
    auto *add = new QPushButton("Add Employee");
    auto *edit = new QPushButton("Edit");
    auto *del = new QPushButton("Delete");
    bar->addWidget(add); bar->addWidget(edit); bar->addWidget(del);
    bar->addStretch();
    layout->addLayout(bar);

    m_table = new QTableWidget;
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Name", "Designation", "Phone", "Email", "Active"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    layout->addWidget(m_table);

    connect(add, &QPushButton::clicked, this, [this] {
        EmployeeDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            if (!m_ctx.employees.save(dlg.value()))
                QMessageBox::warning(this, "Error", m_ctx.employees.errorMessage());
            refresh();
        }
    });
    connect(edit, &QPushButton::clicked, this, [this] {
        const qint64 id = selectedId();
        if (!id) return;
        for (const Employee &e : m_ctx.employees.list())
            if (e.id == id) {
                EmployeeDialog dlg(this, e);
                if (dlg.exec() == QDialog::Accepted) {
                    if (!m_ctx.employees.save(dlg.value()))
                        QMessageBox::warning(this, "Error",
                                             m_ctx.employees.errorMessage());
                    refresh();
                }
                break;
            }
    });
    connect(del, &QPushButton::clicked, this, [this] {
        const qint64 id = selectedId();
        if (!id) return;
        if (QMessageBox::question(this, "Delete employee",
                "Delete this employee? Their transactions are kept.")
            != QMessageBox::Yes)
            return;
        if (!m_ctx.employees.remove(id))
            QMessageBox::warning(this, "Error", m_ctx.employees.errorMessage());
        refresh();
    });

    refresh();
}

qint64 EmployeesPage::selectedId() const
{
    const auto items = m_table->selectedItems();
    return items.isEmpty() ? 0 : items.first()->data(Qt::UserRole).toLongLong();
}

void EmployeesPage::refresh()
{
    const QVector<Employee> rows = m_ctx.employees.list();
    m_table->setSortingEnabled(false);
    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        const Employee &e = rows[r];
        const QStringList cells{e.name, e.designation, e.phone, e.email,
                                e.active ? "Yes" : "No"};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            it->setData(Qt::UserRole, e.id);
            m_table->setItem(r, c, it);
        }
    }
    m_table->setSortingEnabled(true);
}

} // namespace eb
