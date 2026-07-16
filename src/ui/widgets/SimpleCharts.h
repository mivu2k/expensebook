#pragma once
#include <QVector>
#include <QWidget>

namespace eb {

// Lightweight QPainter chart widgets used on the Dashboard. They are
// dependency-free (work without Qt Charts) and theme-aware.

struct ChartPoint { QString label; double value = 0; };
struct ChartSeriesPoint { QString label; double a = 0; double b = 0; };

/// Vertical bar chart (single series).
class BarChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget *parent = nullptr);
    void setData(QVector<ChartPoint> data, const QString &title);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVector<ChartPoint> m_data;
    QString m_title;
};

/// Grouped two-series bar chart (used for cash flow: income vs expense).
class DualBarChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit DualBarChartWidget(QWidget *parent = nullptr);
    void setData(QVector<ChartSeriesPoint> data, const QString &title,
                 const QString &labelA, const QString &labelB);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVector<ChartSeriesPoint> m_data;
    QString m_title, m_labelA, m_labelB;
};

/// Pie chart with legend.
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget *parent = nullptr);
    void setData(QVector<ChartPoint> data, const QString &title);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVector<ChartPoint> m_data;
    QString m_title;
};

} // namespace eb
