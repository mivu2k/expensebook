#pragma once
#include <QString>

namespace eb::theme {

/// Applies the named theme ("light" or "dark") application-wide by
/// loading the corresponding QSS resource.
void apply(const QString &name);

} // namespace eb::theme
