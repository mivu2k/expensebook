#include "Money.h"
#include "core/AppSettings.h"
#include <QLocale>
#include <cmath>

namespace eb::money {

QString format(qint64 cents)
{
    const bool neg = cents < 0;
    const qint64 abs = neg ? -cents : cents;
    QString s = QLocale::system().toString(static_cast<qlonglong>(abs / 100));
    return QString("%1%2.%3")
        .arg(neg ? "-" : "", s, QString::number(abs % 100).rightJustified(2, '0'));
}

QString formatWithSymbol(qint64 cents)
{
    return AppSettings::instance().currencySymbol() + format(cents);
}

bool parse(const QString &text, qint64 &centsOut)
{
    QString t = text.trimmed();
    t.remove(AppSettings::instance().currencySymbol());
    t.remove(',').remove(' ');
    bool ok = false;
    const double v = t.toDouble(&ok);
    if (!ok) return false;
    centsOut = static_cast<qint64>(std::llround(v * 100.0));
    return true;
}

} // namespace eb::money
