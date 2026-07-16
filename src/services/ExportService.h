#pragma once
#include "ReportService.h"
#include <QString>

class QWidget;

namespace eb {

/// Exports report rows to CSV / XLSX / PDF and drives printing.
/// The XLSX path requires the QXlsx dependency (EB_HAS_XLSX).
class ExportService {
public:
    bool exportCsv(const QString &file, const QVector<ReportRow> &rows);
    bool exportXlsx(const QString &file, const QVector<ReportRow> &rows);
    bool exportPdf(const QString &file, const QString &title,
                   const QVector<ReportRow> &rows);
    /// Opens the platform print dialog and prints the ledger.
    bool printReport(QWidget *parent, const QString &title,
                     const QVector<ReportRow> &rows);
    QString lastError() const { return m_lastError; }

    /// Builds the accounting-ledger HTML used for both PDF and print.
    static QString ledgerHtml(const QString &title, const QVector<ReportRow> &rows);

private:
    QString m_lastError;
};

} // namespace eb
