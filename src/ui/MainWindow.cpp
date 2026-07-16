#include "MainWindow.h"
#include "core/AppSettings.h"
#include "ui/AppContext.h"
#include "ui/pages/BackupPage.h"
#include "ui/pages/CategoriesPage.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/EmployeesPage.h"
#include "ui/pages/ExpensePage.h"
#include "ui/pages/IncomePage.h"
#include "ui/pages/ReportsPage.h"
#include "ui/pages/SettingsPage.h"
#include "ui/pages/SubCategoriesPage.h"
#include "utils/Theme.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>

namespace eb {

MainWindow::MainWindow(AppContext &ctx, QWidget *parent)
    : QMainWindow(parent), m_ctx(ctx)
{
    setWindowTitle(AppSettings::instance().companyName() + " — ExpenseBook");
    resize(1280, 800);

    auto *central = new QWidget;
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ----- Navigation sidebar -----
    auto *side = new QWidget;
    side->setObjectName("sidebar");
    side->setFixedWidth(210);
    auto *sideLayout = new QVBoxLayout(side);
    auto *brand = new QLabel("ExpenseBook");
    brand->setObjectName("brand");
    sideLayout->addWidget(brand);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("Search…  (Ctrl+F)");
    m_search->setClearButtonEnabled(true);
    sideLayout->addWidget(m_search);

    m_nav = new QListWidget;
    m_nav->setObjectName("nav");
    for (const QString &page :
         {"📊  Dashboard", "💸  Expenses", "💰  Income", "🏷  Categories",
          "🗂  Sub Categories", "👤  Employees", "📑  Reports", "⚙  Settings",
          "🗄  Backup & Restore"})
        m_nav->addItem(page);
    sideLayout->addWidget(m_nav, 1);
    root->addWidget(side);

    // ----- Pages -----
    m_pages = new QStackedWidget;
    m_pages->addWidget(m_dashboard = new DashboardPage(ctx));
    m_pages->addWidget(m_expensePage = new ExpensePage(ctx));
    m_pages->addWidget(m_incomePage = new IncomePage(ctx));
    auto *categoriesPage = new CategoriesPage(ctx);
    auto *subCategoriesPage = new SubCategoriesPage(ctx);
    auto *employeesPage = new EmployeesPage(ctx);
    auto *reportsPage = new ReportsPage(ctx);
    auto *settingsPage = new SettingsPage(ctx);
    auto *backupPage = new BackupPage(ctx);
    m_pages->addWidget(categoriesPage);
    m_pages->addWidget(subCategoriesPage);
    m_pages->addWidget(employeesPage);
    m_pages->addWidget(reportsPage);
    m_pages->addWidget(settingsPage);
    m_pages->addWidget(backupPage);
    root->addWidget(m_pages, 1);

    setCentralWidget(central);
    statusBar()->showMessage("Ready");

    connect(m_nav, &QListWidget::currentRowChanged, this, [this](int row) {
        m_pages->setCurrentIndex(row);
        // Pages that depend on lookup tables refresh lazily on entry.
        if (auto *w = m_pages->currentWidget())
            QMetaObject::invokeMethod(w, "refresh");
    });
    m_nav->setCurrentRow(0);

    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::onGlobalSearch);
    connect(settingsPage, &SettingsPage::themeChanged, this,
            [](const QString &t) { theme::apply(t); });
    connect(settingsPage, &SettingsPage::settingsSaved, this, [this] {
        setWindowTitle(AppSettings::instance().companyName() + " — ExpenseBook");
    });
    connect(&m_ctx.undo, &UndoService::availabilityChanged, this, [this](bool can) {
        statusBar()->showMessage(can ? "A deleted record can be restored with Ctrl+Z"
                                     : "Ready");
    });

    buildShortcuts();
}

void MainWindow::buildShortcuts()
{
    // Ctrl+1..9 jump to pages.
    for (int i = 0; i < 9; ++i) {
        auto *sc = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(i + 1)), this);
        connect(sc, &QShortcut::activated, this, [this, i] { m_nav->setCurrentRow(i); });
    }
    // Ctrl+N: new expense
    connect(new QShortcut(QKeySequence::New, this), &QShortcut::activated, this, [this] {
        m_nav->setCurrentRow(1);
    });
    // Ctrl+F: focus search
    connect(new QShortcut(QKeySequence::Find, this), &QShortcut::activated, this, [this] {
        m_search->setFocus();
        m_search->selectAll();
    });
    // Ctrl+S: save the expense form when it is visible
    connect(new QShortcut(QKeySequence::Save, this), &QShortcut::activated, this, [this] {
        if (m_pages->currentWidget() == m_expensePage)
            m_expensePage->saveCurrent();
    });
    // Ctrl+Z: undo delete
    connect(new QShortcut(QKeySequence::Undo, this), &QShortcut::activated,
            this, &MainWindow::onUndoDelete);
}

void MainWindow::onGlobalSearch(const QString &text)
{
    m_expensePage->setSearchText(text);
    m_incomePage->setSearchText(text);
    // Jump to the expenses page so results are visible immediately.
    if (!text.isEmpty() && m_pages->currentIndex() != 1 && m_pages->currentIndex() != 2)
        m_nav->setCurrentRow(1);
}

void MainWindow::onUndoDelete()
{
    // Each controller restores only its own record type; try both.
    if (m_ctx.expenses.undoDelete() || m_ctx.income.undoDelete()) {
        m_expensePage->refresh();
        m_incomePage->refresh();
        statusBar()->showMessage("Deleted record restored.", 4000);
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    const auto &s = AppSettings::instance();
    if (s.autoBackupEnabled())
        m_ctx.backup.createBackup(s.backupPath());
    e->accept();
}

} // namespace eb
