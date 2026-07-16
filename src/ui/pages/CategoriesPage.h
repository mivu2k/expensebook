#pragma once
#include <QWidget>

class QTableWidget;

namespace eb {

struct AppContext;

/// Category CRUD with a table + toolbar-style buttons.
class CategoriesPage : public QWidget {
    Q_OBJECT
public:
    explicit CategoriesPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    qint64 selectedId() const;

    AppContext &m_ctx;
    QTableWidget *m_table;
};

} // namespace eb
