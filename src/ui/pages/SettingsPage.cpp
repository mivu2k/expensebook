#include "SettingsPage.h"
#include "core/AppSettings.h"
#include "ui/AppContext.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinterInfo>
#include <QPushButton>
#include <QVBoxLayout>

namespace eb {

SettingsPage::SettingsPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QVBoxLayout(this);
    auto *box = new QGroupBox("Application Settings");
    box->setMaximumWidth(560);
    auto *form = new QFormLayout(box);

    m_theme = new QComboBox;
    m_theme->addItem("Light", "light");
    m_theme->addItem("Dark", "dark");
    form->addRow("Theme", m_theme);

    form->addRow("Currency symbol", m_currency = new QLineEdit);
    m_currency->setMaxLength(4);
    form->addRow("Company name", m_company = new QLineEdit);

    auto *logoRow = new QHBoxLayout;
    logoRow->addWidget(m_logo = new QLineEdit, 1);
    auto *browseLogo = new QPushButton("…");
    logoRow->addWidget(browseLogo);
    form->addRow("Logo file", logoRow);

    auto *backupRow = new QHBoxLayout;
    backupRow->addWidget(m_backupPath = new QLineEdit, 1);
    auto *browseBackup = new QPushButton("…");
    backupRow->addWidget(browseBackup);
    form->addRow("Backup folder", backupRow);

    m_printer = new QComboBox;
    m_printer->addItem("(system default)", "");
    for (const QPrinterInfo &p : QPrinterInfo::availablePrinters())
        m_printer->addItem(p.printerName(), p.printerName());
    form->addRow("Default printer", m_printer);

    form->addRow("", m_autoBackup = new QCheckBox("Auto-backup on exit"));

    auto *save = new QPushButton("Save Settings");
    form->addRow(save);
    layout->addWidget(box);
    layout->addStretch();

    connect(browseLogo, &QPushButton::clicked, this, [this] {
        const QString f = QFileDialog::getOpenFileName(this, "Choose logo", QString(),
                                                       "Images (*.png *.jpg *.jpeg)");
        if (!f.isEmpty()) m_logo->setText(f);
    });
    connect(browseBackup, &QPushButton::clicked, this, [this] {
        const QString d = QFileDialog::getExistingDirectory(this, "Choose backup folder");
        if (!d.isEmpty()) m_backupPath->setText(d);
    });
    connect(save, &QPushButton::clicked, this, [this] { this->save(); });

    load();
}

void SettingsPage::load()
{
    const auto &s = AppSettings::instance();
    m_theme->setCurrentIndex(qMax(0, m_theme->findData(s.theme())));
    m_currency->setText(s.currencySymbol());
    m_company->setText(s.companyName());
    m_logo->setText(s.logoPath());
    m_backupPath->setText(s.backupPath());
    m_printer->setCurrentIndex(qMax(0, m_printer->findData(s.defaultPrinter())));
    m_autoBackup->setChecked(s.autoBackupEnabled());
}

void SettingsPage::save()
{
    auto &s = AppSettings::instance();
    s.setTheme(m_theme->currentData().toString());
    s.setCurrencySymbol(m_currency->text().isEmpty() ? "$" : m_currency->text());
    s.setCompanyName(m_company->text().trimmed());
    s.setLogoPath(m_logo->text().trimmed());
    s.setBackupPath(m_backupPath->text().trimmed());
    s.setDefaultPrinter(m_printer->currentData().toString());
    s.setAutoBackupEnabled(m_autoBackup->isChecked());

    emit themeChanged(s.theme());
    emit settingsSaved();
    QMessageBox::information(this, "Settings", "Settings saved.");
}

} // namespace eb
