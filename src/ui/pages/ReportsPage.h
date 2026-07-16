#pragma once
#include "services/ReportService.h"
#include <QWidget>

class QComboBox;
class QDateEdit;
class QLineEdit;
class QLabel;
class QTableWidget;

namespace eb {

struct AppContext;

/// Report builder: period presets, filters, results grid and
/// CSV / XLSX / PDF export plus printing.
class ReportsPage : public QWidget {
    Q_OBJECT
public:
    explicit ReportsPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();      // reload filter lookups
    void runReport();

private:
    QString reportTitle() const;
    void applyPeriodPreset(int index);

    AppContext &m_ctx;
    QComboBox *m_period, *m_employee, *m_category, *m_subCategory, *m_method;
    QDateEdit *m_from, *m_to;
    QLineEdit *m_minAmount, *m_maxAmount, *m_text;
    QLabel *m_total;
    QTableWidget *m_table;
    QVector<ReportRow> m_rows;
};

} // namespace eb
