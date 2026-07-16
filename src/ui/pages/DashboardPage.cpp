#include "DashboardPage.h"
#include "ui/AppContext.h"
#include "ui/widgets/SimpleCharts.h"
#include "utils/Money.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

namespace eb {

DashboardPage::DashboardPage(AppContext &ctx, QWidget *parent)
    : QWidget(parent), m_ctx(ctx)
{
    auto *layout = new QGridLayout(this);

    // KPI cards row
    auto *cards = new QHBoxLayout;
    cards->addWidget(makeCard("This Month — Expenses", m_monthExpense));
    cards->addWidget(makeCard("This Month — Income", m_monthIncome));
    cards->addWidget(makeCard("This Year — Expenses", m_yearExpense));
    cards->addWidget(makeCard("This Month — Net Cash", m_netCash));
    layout->addLayout(cards, 0, 0, 1, 2);

    layout->addWidget(m_monthlyChart = new BarChartWidget, 1, 0);
    layout->addWidget(m_pieChart = new PieChartWidget, 1, 1);
    layout->addWidget(m_yearlyChart = new BarChartWidget, 2, 0);
    layout->addWidget(m_cashFlowChart = new DualBarChartWidget, 2, 1);
    layout->setRowStretch(1, 1);
    layout->setRowStretch(2, 1);

    refresh();
}

QLabel *DashboardPage::makeCard(const QString &title, QLabel *&valueLabel)
{
    auto *card = new QLabel;
    card->setObjectName("kpiCard");
    card->setMinimumHeight(72);
    auto *v = new QVBoxLayout(card);
    auto *t = new QLabel(title);
    t->setObjectName("kpiTitle");
    valueLabel = new QLabel("—");
    valueLabel->setObjectName("kpiValue");
    v->addWidget(t);
    v->addWidget(valueLabel);
    return card;
}

void DashboardPage::refresh()
{
    auto [mFrom, mTo] = Period::monthly();
    auto [yFrom, yTo] = Period::yearly();

    TransactionFilter month; month.from = mFrom; month.to = mTo;
    TransactionFilter year;  year.from = yFrom;  year.to = yTo;

    ExpenseRepository expenses(m_ctx.db.connection());
    IncomeRepository income(m_ctx.db.connection());

    const qint64 mExp = expenses.totalCents(month);
    const qint64 mInc = income.totalCents(month);
    m_monthExpense->setText(money::formatWithSymbol(mExp));
    m_monthIncome->setText(money::formatWithSymbol(mInc));
    m_yearExpense->setText(money::formatWithSymbol(expenses.totalCents(year)));
    m_netCash->setText(money::formatWithSymbol(mInc - mExp));

    ReportService &reports = m_ctx.reports.service();

    QVector<ChartPoint> monthly;
    QVector<ChartSeriesPoint> cashFlow;
    for (const MonthValue &mv : reports.monthlyTotals(12)) {
        const QString label = mv.month.mid(5); // "MM"
        monthly.append({label, mv.expenseCents / 100.0});
        cashFlow.append({label, mv.incomeCents / 100.0, mv.expenseCents / 100.0});
    }
    m_monthlyChart->setData(monthly, "Monthly Expenses (last 12 months)");
    m_cashFlowChart->setData(cashFlow, "Cash Flow", "Income", "Expense");

    QVector<ChartPoint> pie;
    for (const NameValue &nv : reports.expensesByCategory(yFrom, yTo))
        pie.append({nv.name, nv.cents / 100.0});
    m_pieChart->setData(pie, "Expenses by Category (this year)");

    QVector<ChartPoint> yearly;
    for (const NameValue &nv : reports.yearlyExpenseTotals())
        yearly.append({nv.name, nv.cents / 100.0});
    m_yearlyChart->setData(yearly, "Yearly Expense Trend");
}

} // namespace eb
