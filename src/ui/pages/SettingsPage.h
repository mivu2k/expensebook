#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QCheckBox;

namespace eb {

struct AppContext;

/// Application preferences: theme, currency, company identity, backup
/// path, default printer and auto-backup.
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(AppContext &ctx, QWidget *parent = nullptr);

signals:
    void themeChanged(const QString &theme);
    void settingsSaved();

private:
    void load();
    void save();

    AppContext &m_ctx;
    QComboBox *m_theme, *m_printer;
    QLineEdit *m_currency, *m_company, *m_logo, *m_backupPath;
    QCheckBox *m_autoBackup;
};

} // namespace eb
