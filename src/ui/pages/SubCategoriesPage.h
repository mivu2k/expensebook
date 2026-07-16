#pragma once
#include <QWidget>

class QComboBox;
class QListWidget;
class QLineEdit;

namespace eb {

struct AppContext;

/// Sub-category CRUD grouped under a selected parent category.
class SubCategoriesPage : public QWidget {
    Q_OBJECT
public:
    explicit SubCategoriesPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    void reloadList();

    AppContext &m_ctx;
    QComboBox *m_category;
    QLineEdit *m_newName;
    QListWidget *m_list;
};

} // namespace eb
