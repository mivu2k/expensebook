#include "SimpleCharts.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace eb {

namespace {
const QVector<QColor> kPalette = {
    QColor("#2b579a"), QColor("#c0504d"), QColor("#9bbb59"), QColor("#8064a2"),
    QColor("#4bacc6"), QColor("#f79646"), QColor("#7f7f7f"), QColor("#1f6f43"),
    QColor("#bf9000"), QColor("#5b2c6f"), QColor("#148f77"), QColor("#a04000"),
};

void drawTitle(QPainter &p, const QRect &r, const QString &title, const QPalette &pal)
{
    // The app stylesheet sizes fonts in pixels, so adjust pixelSize
    // (pointSizeF() is -1 for pixel-sized fonts).
    const QFont base = p.font();
    QFont f = base;
    f.setBold(true);
    if (f.pixelSize() > 0)
        f.setPixelSize(f.pixelSize() + 2);
    else
        f.setPointSizeF(qMax(1.0, f.pointSizeF() + 1));
    p.setFont(f);
    p.setPen(pal.color(QPalette::WindowText));
    p.drawText(QRect(r.left(), r.top(), r.width(), 24), Qt::AlignCenter, title);
    p.setFont(base);
}
} // namespace

// ---------- BarChartWidget ----------

BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(220);
}

void BarChartWidget::setData(QVector<ChartPoint> data, const QString &title)
{
    m_data = std::move(data);
    m_title = title;
    update();
}

void BarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawTitle(p, rect(), m_title, palette());
    if (m_data.isEmpty())
        return;

    const QRect plot = rect().adjusted(8, 30, -8, -28);
    double maxV = 0;
    for (const auto &d : m_data) maxV = qMax(maxV, d.value);
    if (maxV <= 0) maxV = 1;

    const int n = m_data.size();
    const double slot = double(plot.width()) / n;
    const double barW = qMax(4.0, slot * 0.62);
    for (int i = 0; i < n; ++i) {
        const double h = plot.height() * (m_data[i].value / maxV);
        QRectF bar(plot.left() + i * slot + (slot - barW) / 2,
                   plot.bottom() - h, barW, h);
        p.fillRect(bar, kPalette[0]);
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(QRectF(plot.left() + i * slot, plot.bottom() + 4, slot, 20),
                   Qt::AlignHCenter | Qt::AlignTop, m_data[i].label);
    }
}

// ---------- DualBarChartWidget ----------

DualBarChartWidget::DualBarChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(220);
}

void DualBarChartWidget::setData(QVector<ChartSeriesPoint> data, const QString &title,
                                 const QString &labelA, const QString &labelB)
{
    m_data = std::move(data);
    m_title = title;
    m_labelA = labelA;
    m_labelB = labelB;
    update();
}

void DualBarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawTitle(p, rect(), m_title, palette());
    if (m_data.isEmpty())
        return;

    // Legend
    p.setPen(palette().color(QPalette::WindowText));
    p.fillRect(10, 28, 10, 10, kPalette[7]);
    p.drawText(24, 38, m_labelA);
    p.fillRect(110, 28, 10, 10, kPalette[1]);
    p.drawText(124, 38, m_labelB);

    const QRect plot = rect().adjusted(8, 48, -8, -28);
    double maxV = 1;
    for (const auto &d : m_data) maxV = qMax(maxV, qMax(d.a, d.b));

    const int n = m_data.size();
    const double slot = double(plot.width()) / n;
    const double barW = qMax(3.0, slot * 0.3);
    for (int i = 0; i < n; ++i) {
        const double hA = plot.height() * (m_data[i].a / maxV);
        const double hB = plot.height() * (m_data[i].b / maxV);
        const double x = plot.left() + i * slot + slot / 2;
        p.fillRect(QRectF(x - barW, plot.bottom() - hA, barW, hA), kPalette[7]);
        p.fillRect(QRectF(x, plot.bottom() - hB, barW, hB), kPalette[1]);
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(QRectF(plot.left() + i * slot, plot.bottom() + 4, slot, 20),
                   Qt::AlignHCenter | Qt::AlignTop, m_data[i].label);
    }
}

// ---------- PieChartWidget ----------

PieChartWidget::PieChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(220);
}

void PieChartWidget::setData(QVector<ChartPoint> data, const QString &title)
{
    m_data = std::move(data);
    m_title = title;
    update();
}

void PieChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawTitle(p, rect(), m_title, palette());
    if (m_data.isEmpty())
        return;

    double total = 0;
    for (const auto &d : m_data) total += d.value;
    if (total <= 0)
        return;

    const int side = qMin(width() / 2, height() - 40);
    const QRectF pieRect(16, 36, side, side);
    double start = 90 * 16;
    for (int i = 0; i < m_data.size(); ++i) {
        const double span = -360.0 * 16 * (m_data[i].value / total);
        p.setBrush(kPalette[i % kPalette.size()]);
        p.setPen(Qt::NoPen);
        p.drawPie(pieRect, int(start), int(span));
        start += span;
    }

    // Legend on the right, truncated to the widget height.
    int y = 40;
    p.setPen(palette().color(QPalette::WindowText));
    for (int i = 0; i < m_data.size() && y < height() - 12; ++i) {
        p.fillRect(int(pieRect.right()) + 16, y, 10, 10, kPalette[i % kPalette.size()]);
        const int pct = int(std::lround(100.0 * m_data[i].value / total));
        p.drawText(int(pieRect.right()) + 32, y + 9,
                   QString("%1 (%2%)").arg(m_data[i].label).arg(pct));
        y += 18;
    }
}

} // namespace eb
