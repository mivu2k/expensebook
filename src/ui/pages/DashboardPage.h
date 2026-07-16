#pragma once
#include <QWidget>

class QLabel;

namespace eb {

struct AppContext;
class BarChartWidget;
class DualBarChartWidget;
class PieChartWidget;

/// Landing page: KPI cards plus the four charts (monthly expense,
/// category pie, yearly trend, cash flow).
class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(AppContext &ctx, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    QLabel *makeCard(const QString &title, QLabel *&valueLabel);

    AppContext &m_ctx;
    QLabel *m_monthExpense = nullptr, *m_monthIncome = nullptr,
           *m_yearExpense = nullptr, *m_netCash = nullptr;
    BarChartWidget *m_monthlyChart = nullptr;
    PieChartWidget *m_pieChart = nullptr;
    BarChartWidget *m_yearlyChart = nullptr;
    DualBarChartWidget *m_cashFlowChart = nullptr;
};

} // namespace eb
