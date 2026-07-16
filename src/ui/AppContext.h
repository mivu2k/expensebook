#pragma once
#include "controllers/CategoryController.h"
#include "controllers/EmployeeController.h"
#include "controllers/ExpenseController.h"
#include "controllers/IncomeController.h"
#include "controllers/ReportController.h"
#include "core/Database.h"
#include "repositories/PaymentMethodRepository.h"
#include "services/AuditService.h"
#include "services/BackupService.h"
#include "services/UndoService.h"

namespace eb {

/// Composition root: owns every controller/service and is passed by
/// reference to the pages (dependency injection without a framework).
struct AppContext {
    explicit AppContext(Database &database)
        : db(database),
          audit(database.connection()),
          backup(database.connection(), database.filePath()),
          paymentMethods(database.connection()),
          expenses(database.connection(), audit, undo),
          income(database.connection(), audit, undo),
          categories(database.connection(), audit),
          employees(database.connection(), audit),
          reports(database.connection())
    {
    }

    Database &db;
    AuditService audit;
    UndoService undo;
    BackupService backup;
    PaymentMethodRepository paymentMethods;
    ExpenseController expenses;
    IncomeController income;
    CategoryController categories;
    EmployeeController employees;
    ReportController reports;
};

} // namespace eb
