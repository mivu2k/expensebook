#include "AppSettings.h"
#include <QStandardPaths>

namespace eb {

AppSettings::AppSettings() : m_s("ExpenseBook", "ExpenseBook") {}

AppSettings &AppSettings::instance()
{
    static AppSettings s;
    return s;
}

static QString dataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString AppSettings::databasePath() const
{ return m_s.value("db/path", dataDir() + "/expensebook.db").toString(); }
void AppSettings::setDatabasePath(const QString &p) { m_s.setValue("db/path", p); }

QString AppSettings::theme() const { return m_s.value("ui/theme", "light").toString(); }
void AppSettings::setTheme(const QString &t) { m_s.setValue("ui/theme", t); }

QString AppSettings::currencySymbol() const { return m_s.value("ui/currency", "$").toString(); }
void AppSettings::setCurrencySymbol(const QString &c) { m_s.setValue("ui/currency", c); }

QString AppSettings::companyName() const
{ return m_s.value("company/name", "My Company").toString(); }
void AppSettings::setCompanyName(const QString &n) { m_s.setValue("company/name", n); }

QString AppSettings::logoPath() const { return m_s.value("company/logo").toString(); }
void AppSettings::setLogoPath(const QString &p) { m_s.setValue("company/logo", p); }

QString AppSettings::backupPath() const
{ return m_s.value("backup/path", dataDir() + "/backups").toString(); }
void AppSettings::setBackupPath(const QString &p) { m_s.setValue("backup/path", p); }

QString AppSettings::defaultPrinter() const { return m_s.value("print/printer").toString(); }
void AppSettings::setDefaultPrinter(const QString &p) { m_s.setValue("print/printer", p); }

bool AppSettings::autoBackupEnabled() const { return m_s.value("backup/auto", true).toBool(); }
void AppSettings::setAutoBackupEnabled(bool on) { m_s.setValue("backup/auto", on); }

} // namespace eb
