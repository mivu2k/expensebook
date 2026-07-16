#pragma once
#include <QString>

namespace eb::money {

/// Formats integer cents as "1,234.56" (no currency symbol).
QString format(qint64 cents);

/// Formats with the configured currency symbol, e.g. "$1,234.56".
QString formatWithSymbol(qint64 cents);

/// Parses user input like "1,234.56" into cents. Returns false on
/// invalid input.
bool parse(const QString &text, qint64 &centsOut);

} // namespace eb::money
