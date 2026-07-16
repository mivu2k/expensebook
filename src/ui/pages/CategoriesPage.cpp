#include "CategoriesPage.h"
#include "ui/AppContext.h"
#include "ui/dialogs/CategoryDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace eb {

CategoriesPage::CategoriesPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *bar = new QHBoxLayout;
    auto *add = new QPushButton("Add Category");
    auto *edit = new QPushButton("Edit");
    auto *del = new QPushButton("Delete");
    bar->addWidget(add); bar->addWidget(edit); bar->addWidget(del);
    bar->addStretch();
    layout->addLayout(bar);

    m_table = new QTableWidget;
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Name", "Used for", "Sub Categories"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    layout->addWidget(m_table);

    connect(add, &QPushButton::clicked, this, [this] {
        CategoryDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            if (!m_ctx.categories.saveCategory(dlg.value()))
                QMessageBox::warning(this, "Error", m_ctx.categories.errorMessage());
            refresh();
        }
    });
    connect(edit, &QPushButton::clicked, this, [this] {
        const qint64 id = selectedId();
        if (!id) return;
        for (const Category &c : m_ctx.categories.categories())
            if (c.id == id) {
                CategoryDialog dlg(this, c);
                if (dlg.exec() == QDialog::Accepted) {
                    if (!m_ctx.categories.saveCategory(dlg.value()))
                        QMessageBox::warning(this, "Error",
                                             m_ctx.categories.errorMessage());
                    refresh();
                }
                break;
            }
    });
    connect(del, &QPushButton::clicked, this, [this] {
        const qint64 id = selectedId();
        if (!id) return;
        if (QMessageBox::question(this, "Delete category",
                "Delete this category and all its sub-categories?\n"
                "Existing transactions keep their data but lose the link.")
            != QMessageBox::Yes)
            return;
        if (!m_ctx.categories.removeCategory(id))
            QMessageBox::warning(this, "Error", m_ctx.categories.errorMessage());
        refresh();
    });

    refresh();
}

qint64 CategoriesPage::selectedId() const
{
    const auto items = m_table->selectedItems();
    return items.isEmpty() ? 0 : items.first()->data(Qt::UserRole).toLongLong();
}

void CategoriesPage::refresh()
{
    const QVector<Category> cats = m_ctx.categories.categories();
    m_table->setSortingEnabled(false);
    m_table->setRowCount(cats.size());
    for (int r = 0; r < cats.size(); ++r) {
        const int subCount = m_ctx.categories.subCategories(cats[r].id).size();
        const QStringList cells{cats[r].name, cats[r].kind,
                                QString::number(subCount)};
        for (int c = 0; c < cells.size(); ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            it->setData(Qt::UserRole, cats[r].id);
            m_table->setItem(r, c, it);
        }
    }
    m_table->setSortingEnabled(true);
}

} // namespace eb
