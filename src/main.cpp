#include "core/AppSettings.h"
#include "core/Database.h"
#include "ui/AppContext.h"
#include "ui/MainWindow.h"
#include "utils/Theme.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("ExpenseBook");
    QApplication::setOrganizationName("ExpenseBook");

    eb::Database database;
    if (!database.open(eb::AppSettings::instance().databasePath())) {
        qCritical().noquote() << "Database error:" << database.lastError();
        QMessageBox::critical(nullptr, "Database error",
                              "Could not open the database:\n" + database.lastError());
        return 1;
    }

    eb::theme::apply(eb::AppSettings::instance().theme());

    eb::AppContext context(database);
    eb::MainWindow window(context);
    window.show();

    const int rc = app.exec();
    database.close();
    return rc;
}
