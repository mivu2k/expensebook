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
class AttachmentListWidget;

/// Expense entry form plus the searchable/sortable expense table.
class ExpensePage : public QWidget {
    Q_OBJECT
public:
    explicit ExpensePage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();                       // reload lookups + table
    void setSearchText(const QString &text);
    void saveCurrent();                   // Ctrl+S entry point

private slots:
    void onSave(bool andNew);
    void onClear();
    void onDelete();
    void onDuplicate();
    void onRowSelected();

private:
    Expense collect() const;
    void loadIntoForm(const Expense &e);
    void reloadLookups();
    void reloadTable();
    void persistAttachments(qint64 expenseId);

    AppContext &m_ctx;
    qint64 m_editingId = 0;

    QDateEdit *m_date;
    QComboBox *m_employee, *m_category, *m_subCategory, *m_method;
    QLineEdit *m_description, *m_amount, *m_reference;
    QPlainTextEdit *m_remarks;
    AttachmentListWidget *m_attachments;
    QTableWidget *m_table;
    QString m_search;
};

} // namespace eb
