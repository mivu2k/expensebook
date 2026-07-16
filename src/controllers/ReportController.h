#pragma once
#include "services/ExportService.h"
#include "services/ReportService.h"
#include <QObject>

namespace eb {

/// Wires report queries to export/print actions for the Reports page.
class ReportController : public QObject {
    Q_OBJECT
public:
    explicit ReportController(QSqlDatabase db, QObject *parent = nullptr)
        : QObject(parent), m_reports(db) {}

    QVector<ReportRow> run(const TransactionFilter &f) const
    { return m_reports.expenseReport(f); }

    bool exportCsv(const QString &file, const QVector<ReportRow> &rows)
    { return m_export.exportCsv(file, rows); }
    bool exportXlsx(const QString &file, const QVector<ReportRow> &rows)
    { return m_export.exportXlsx(file, rows); }
    bool exportPdf(const QString &file, const QString &title,
                   const QVector<ReportRow> &rows)
    { return m_export.exportPdf(file, title, rows); }
    bool print(QWidget *parent, const QString &title, const QVector<ReportRow> &rows)
    { return m_export.printReport(parent, title, rows); }

    ReportService &service() { return m_reports; }
    QString errorMessage() const { return m_export.lastError(); }

private:
    ReportService m_reports;
    ExportService m_export;
};

} // namespace eb
