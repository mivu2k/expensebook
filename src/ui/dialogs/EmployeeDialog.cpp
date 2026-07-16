#include "EmployeeDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

namespace eb {

EmployeeDialog::EmployeeDialog(QWidget *parent, const Employee &initial)
    : QDialog(parent), m_id(initial.id)
{
    setWindowTitle(initial.id ? "Edit Employee" : "New Employee");
    auto *form = new QFormLayout(this);
    form->addRow("Name*", m_name = new QLineEdit(initial.name));
    form->addRow("Designation", m_designation = new QLineEdit(initial.designation));
    form->addRow("Phone", m_phone = new QLineEdit(initial.phone));
    form->addRow("Email", m_email = new QLineEdit(initial.email));
    form->addRow("", m_active = new QCheckBox("Active"));
    m_active->setChecked(initial.active);

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

Employee EmployeeDialog::value() const
{
    return {m_id, m_name->text().trimmed(), m_designation->text().trimmed(),
            m_phone->text().trimmed(), m_email->text().trimmed(),
            m_active->isChecked()};
}

} // namespace eb
