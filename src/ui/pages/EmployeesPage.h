#pragma once
#include <QWidget>

class QTableWidget;

namespace eb {

struct AppContext;

class EmployeesPage : public QWidget {
    Q_OBJECT
public:
    explicit EmployeesPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    qint64 selectedId() const;

    AppContext &m_ctx;
    QTableWidget *m_table;
};

} // namespace eb
