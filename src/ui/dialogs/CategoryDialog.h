#pragma once
#include "models/Models.h"
#include <QDialog>

class QComboBox;
class QLineEdit;

namespace eb {

/// Modal add/edit dialog for a category (name + kind).
class CategoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit CategoryDialog(QWidget *parent = nullptr, const Category &initial = {});
    Category value() const;

private:
    QLineEdit *m_name;
    QComboBox *m_kind;
    qint64 m_id;
};

} // namespace eb
