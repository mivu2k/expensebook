#pragma once
#include <QWidget>

class QTableWidget;

namespace eb {

struct AppContext;

/// Manual backup / restore plus the backup history list.
class BackupPage : public QWidget {
    Q_OBJECT
public:
    explicit BackupPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    AppContext &m_ctx;
    QTableWidget *m_table;
};

} // namespace eb
