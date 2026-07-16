#include "CategoryDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

namespace eb {

CategoryDialog::CategoryDialog(QWidget *parent, const Category &initial)
    : QDialog(parent), m_id(initial.id)
{
    setWindowTitle(initial.id ? "Edit Category" : "New Category");
    auto *form = new QFormLayout(this);
    form->addRow("Name*", m_name = new QLineEdit(initial.name));
    m_kind = new QComboBox;
    m_kind->addItem("Expense", "expense");
    m_kind->addItem("Income", "income");
    m_kind->addItem("Both", "both");
    m_kind->setCurrentIndex(qMax(0, m_kind->findData(initial.kind)));
    form->addRow("Used for", m_kind);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Name is required.");
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Category CategoryDialog::value() const
{
    return {m_id, m_name->text().trimmed(), m_kind->currentData().toString()};
}

} // namespace eb
