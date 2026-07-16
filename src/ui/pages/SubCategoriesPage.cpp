#include "SubCategoriesPage.h"
#include "ui/AppContext.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace eb {

SubCategoriesPage::SubCategoriesPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel("Category:"));
    top->addWidget(m_category = new QComboBox, 1);
    layout->addLayout(top);

    auto *addRow = new QHBoxLayout;
    addRow->addWidget(m_newName = new QLineEdit, 1);
    m_newName->setPlaceholderText("New sub-category name…");
    auto *add = new QPushButton("Add");
    auto *rename = new QPushButton("Rename");
    auto *del = new QPushButton("Delete");
    addRow->addWidget(add); addRow->addWidget(rename); addRow->addWidget(del);
    layout->addLayout(addRow);

    layout->addWidget(m_list = new QListWidget, 1);
    m_list->setSortingEnabled(true);

    connect(m_category, &QComboBox::currentIndexChanged,
            this, &SubCategoriesPage::reloadList);
    connect(add, &QPushButton::clicked, this, [this] {
        SubCategory s;
        s.categoryId = m_category->currentData().toLongLong();
        s.name = m_newName->text();
        if (!m_ctx.categories.saveSubCategory(s)) {
            QMessageBox::warning(this, "Error", m_ctx.categories.errorMessage());
            return;
        }
        m_newName->clear();
        reloadList();
    });
    connect(rename, &QPushButton::clicked, this, [this] {
        auto *it = m_list->currentItem();
        if (!it) return;
        bool ok = false;
        const QString name = QInputDialog::getText(this, "Rename sub-category",
            "New name:", QLineEdit::Normal, it->text(), &ok);
        if (!ok || name.trimmed().isEmpty()) return;
        SubCategory s{it->data(Qt::UserRole).toLongLong(),
                      m_category->currentData().toLongLong(), name.trimmed()};
        if (!m_ctx.categories.saveSubCategory(s))
            QMessageBox::warning(this, "Error", m_ctx.categories.errorMessage());
        reloadList();
    });
    connect(del, &QPushButton::clicked, this, [this] {
        auto *it = m_list->currentItem();
        if (!it) return;
        if (QMessageBox::question(this, "Delete", "Delete \"" + it->text() + "\"?")
            != QMessageBox::Yes)
            return;
        if (!m_ctx.categories.removeSubCategory(it->data(Qt::UserRole).toLongLong()))
            QMessageBox::warning(this, "Error", m_ctx.categories.errorMessage());
        reloadList();
    });

    refresh();
}

void SubCategoriesPage::refresh()
{
    const qint64 keep = m_category->currentData().toLongLong();
    m_category->blockSignals(true);
    m_category->clear();
    for (const Category &c : m_ctx.categories.categories())
        m_category->addItem(c.name, c.id);
    const int ix = m_category->findData(keep);
    m_category->setCurrentIndex(ix >= 0 ? ix : 0);
    m_category->blockSignals(false);
    reloadList();
}

void SubCategoriesPage::reloadList()
{
    m_list->clear();
    const qint64 catId = m_category->currentData().toLongLong();
    if (catId <= 0)
        return;
    for (const SubCategory &s : m_ctx.categories.subCategories(catId)) {
        auto *it = new QListWidgetItem(s.name, m_list);
        it->setData(Qt::UserRole, s.id);
    }
}

} // namespace eb
