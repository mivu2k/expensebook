#pragma once
#include "models/Models.h"
#include <QDialog>

class QLineEdit;
class QCheckBox;

namespace eb {

/// Modal add/edit dialog for an employee record.
class EmployeeDialog : public QDialog {
    Q_OBJECT
public:
    explicit EmployeeDialog(QWidget *parent = nullptr, const Employee &initial = {});
    Employee value() const;

private:
    QLineEdit *m_name, *m_designation, *m_phone, *m_email;
    QCheckBox *m_active;
    qint64 m_id;
};

} // namespace eb
