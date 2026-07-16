#include "BackupPage.h"
#include "core/AppSettings.h"
#include "ui/AppContext.h"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace eb {

BackupPage::BackupPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);

    auto *info = new QLabel(
        "Backups are consistent snapshots of the database file. "
        "Restoring replaces the current data and restarts the application.");
    info->setWordWrap(true);
    layout->addWidget(info);

    auto *bar = new QHBoxLayout;
    auto *backupNow = new QPushButton("Backup Now");
    auto *restoreSel = new QPushButton("Restore Selected");
    auto *restoreFile = new QPushButton("Restore From File…");
    bar->addWidget(backupNow); bar->addWidget(restoreSel);
    bar->addWidget(restoreFile); bar->addStretch();
    layout->addLayout(bar);

    m_table = new QTableWidget;
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Created", "File"});
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);

    const auto doRestore = [this](const QString &file) {
        if (QMessageBox::warning(this, "Restore backup",
                "Replace ALL current data with this backup?\n" + file,
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
        if (!m_ctx.backup.restoreBackup(file)) {
            QMessageBox::critical(this, "Restore failed", m_ctx.backup.lastError());
            return;
        }
        QMessageBox::information(this, "Restore complete",
                                 "The application will now restart.");
        QProcess::startDetached(QApplication::applicationFilePath(), {});
        QApplication::quit();
    };

    connect(backupNow, &QPushButton::clicked, this, [this] {
        const QString file =
            m_ctx.backup.createBackup(AppSettings::instance().backupPath());
        if (file.isEmpty())
            QMessageBox::critical(this, "Backup failed", m_ctx.backup.lastError());
        else
            QMessageBox::information(this, "Backup complete", "Saved to:\n" + file);
        refresh();
    });
    connect(restoreSel, &QPushButton::clicked, this, [this, doRestore] {
        const auto items = m_table->selectedItems();
        if (items.isEmpty()) return;
        doRestore(m_table->item(items.first()->row(), 1)->text());
    });
    connect(restoreFile, &QPushButton::clicked, this, [this, doRestore] {
        const QString f = QFileDialog::getOpenFileName(
            this, "Choose backup file", AppSettings::instance().backupPath(),
            "SQLite database (*.db)");
        if (!f.isEmpty()) doRestore(f);
    });

    refresh();
}

void BackupPage::refresh()
{
    const auto history = m_ctx.backup.history();
    m_table->setRowCount(history.size());
    for (int r = 0; r < history.size(); ++r) {
        m_table->setItem(r, 0, new QTableWidgetItem(history[r].createdAt));
        m_table->setItem(r, 1, new QTableWidgetItem(history[r].filePath));
    }
}

} // namespace eb
