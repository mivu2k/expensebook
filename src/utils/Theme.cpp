#include "Theme.h"
#include <QApplication>
#include <QFile>

namespace eb::theme {

void apply(const QString &name)
{
    QFile f(name == "dark" ? ":/styles/dark.qss" : ":/styles/light.qss");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
}

} // namespace eb::theme
