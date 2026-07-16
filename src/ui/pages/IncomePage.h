#pragma once
#include "models/Models.h"
#include <QWidget>

class QComboBox;
class QDateEdit;
class QLineEdit;
class QPlainTextEdit;
class QTableWidget;

namespace eb {

struct AppContext;

/// Income entry form plus the income table.
class IncomePage : public QWidget {
    Q_OBJECT
public:
    explicit IncomePage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();
    void setSearchText(const QString &text);

private slots:
    void onSave(bool andNew);
    void onClear();
    void onDelete();
    void onRowSelected();

private:
    Income collect() const;
    void reloadTable();

    AppContext &m_ctx;
    qint64 m_editingId = 0;
    QDateEdit *m_date;
    QLineEdit *m_source, *m_description, *m_amount, *m_reference;
    QComboBox *m_category, *m_method;
    QPlainTextEdit *m_remarks;
    QTableWidget *m_table;
    QString m_search;
};

} // namespace eb
