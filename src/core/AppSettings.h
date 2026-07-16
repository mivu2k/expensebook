#pragma once
#include <QSettings>
#include <QString>

namespace eb {

/// Typed wrapper over QSettings for the application preferences shown
/// on the Settings page.
class AppSettings {
public:
    static AppSettings &instance();

    QString databasePath() const;
    void setDatabasePath(const QString &p);

    QString theme() const;               // "light" | "dark"
    void setTheme(const QString &t);

    QString currencySymbol() const;      // e.g. "$", "€", "₹"
    void setCurrencySymbol(const QString &c);

    QString companyName() const;
    void setCompanyName(const QString &n);

    QString logoPath() const;
    void setLogoPath(const QString &p);

    QString backupPath() const;
    void setBackupPath(const QString &p);

    QString defaultPrinter() const;
    void setDefaultPrinter(const QString &p);

    bool autoBackupEnabled() const;
    void setAutoBackupEnabled(bool on);

private:
    AppSettings();
    QSettings m_s;
};

} // namespace eb
