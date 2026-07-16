#pragma once
#include "services/ReportService.h"
#include <QWidget>

class QLabel;
class QTableWidget;

namespace eb {

struct AppContext;

/// Spreadsheet-style multi-row expense entry: add any number of rows,
/// see the running total and a live per-category summary, then save
/// them all as expenses in one action.
class BatchEntryPage : public QWidget {
    Q_OBJECT
public:
    explicit BatchEntryPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();      // reload category lookups into row combos

private slots:
    void addRow();
    void recalculate();
    void saveAll();
    void printSheet();

private:
    struct RowData {
        QDate date;
        QString description;
        qint64 categoryId = 0;
        QString categoryName;
        qint64 subCategoryId = 0;
        qint64 amountCents = 0;
    };
    RowData rowData(int row) const;
    QVector<ReportRow> asReportRows() const;

    AppContext &m_ctx;
    QTableWidget *m_grid;
    QTableWidget *m_summary;
    QLabel *m_total;
};

} // namespace eb
