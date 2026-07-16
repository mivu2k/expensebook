#pragma once
#include <QMainWindow>

class QLineEdit;
class QListWidget;
class QStackedWidget;

namespace eb {

struct AppContext;
class DashboardPage;
class ExpensePage;
class IncomePage;

/// Office-style shell: navigation sidebar, global search box, stacked
/// pages, keyboard shortcuts and undo-delete.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(AppContext &ctx, QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onGlobalSearch(const QString &text);
    void onUndoDelete();

private:
    void buildShortcuts();

    AppContext &m_ctx;
    QListWidget *m_nav;
    QStackedWidget *m_pages;
    QLineEdit *m_search;
    DashboardPage *m_dashboard;
    ExpensePage *m_expensePage;
    IncomePage *m_incomePage;
};

} // namespace eb
