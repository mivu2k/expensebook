cmake_minimum_required(VERSION 3.21)
project(ExpenseBook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql PrintSupport)
# Charts is optional; a QPainter-based fallback is used when absent.
find_package(Qt6 QUIET COMPONENTS Charts)

# Optional native .xlsx export via QXlsx (fetched at configure time).
option(EB_WITH_XLSX "Enable native .xlsx export via QXlsx" ON)
if(EB_WITH_XLSX)
    # QXlsx links against Qt6::GuiPrivate; make sure the target exists.
    find_package(Qt6 REQUIRED COMPONENTS Core Gui)
    if(NOT TARGET Qt6::GuiPrivate)
        find_package(Qt6GuiPrivate QUIET)
    endif()
    include(FetchContent)
    FetchContent_Declare(QXlsx
        GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
        GIT_TAG v1.5.1.1
        SOURCE_SUBDIR QXlsx)
    FetchContent_MakeAvailable(QXlsx)
endif()

set(SOURCES
    src/main.cpp
    src/core/Database.cpp
    src/core/MigrationManager.cpp
    src/core/AppSettings.cpp
    src/repositories/EmployeeRepository.cpp
    src/repositories/CategoryRepository.cpp
    src/repositories/SubCategoryRepository.cpp
    src/repositories/PaymentMethodRepository.cpp
    src/repositories/ExpenseRepository.cpp
    src/repositories/IncomeRepository.cpp
    src/repositories/AttachmentRepository.cpp
    src/repositories/AuditLogRepository.cpp
    src/services/AuditService.cpp
    src/services/UndoService.cpp
    src/services/BackupService.cpp
    src/services/ExportService.cpp
    src/services/ReportService.cpp
    src/controllers/ExpenseController.cpp
    src/controllers/IncomeController.cpp
    src/controllers/CategoryController.cpp
    src/controllers/EmployeeController.cpp
    src/controllers/ReportController.cpp
    src/ui/MainWindow.cpp
    src/ui/pages/DashboardPage.cpp
    src/ui/pages/ExpensePage.cpp
    src/ui/pages/BatchEntryPage.cpp
    src/ui/pages/LedgerPage.cpp
    src/ui/pages/IncomePage.cpp
    src/ui/pages/CategoriesPage.cpp
    src/ui/pages/SubCategoriesPage.cpp
    src/ui/pages/EmployeesPage.cpp
    src/ui/pages/ReportsPage.cpp
    src/ui/pages/SettingsPage.cpp
    src/ui/pages/BackupPage.cpp
    src/ui/dialogs/EmployeeDialog.cpp
    src/ui/dialogs/CategoryDialog.cpp
    src/ui/widgets/AttachmentListWidget.cpp
    src/ui/widgets/SimpleCharts.cpp
    src/utils/Money.cpp
    src/utils/Theme.cpp
)

qt_add_executable(ExpenseBook WIN32 MACOSX_BUNDLE ${SOURCES} resources/resources.qrc)

target_include_directories(ExpenseBook PRIVATE src)
target_link_libraries(ExpenseBook PRIVATE Qt6::Widgets Qt6::Sql Qt6::PrintSupport)

if(Qt6Charts_FOUND)
    target_link_libraries(ExpenseBook PRIVATE Qt6::Charts)
    target_compile_definitions(ExpenseBook PRIVATE EB_HAS_CHARTS)
endif()
if(EB_WITH_XLSX)
    target_link_libraries(ExpenseBook PRIVATE QXlsx::QXlsx)
    target_compile_definitions(ExpenseBook PRIVATE EB_HAS_XLSX)
endif()

set_target_properties(ExpenseBook PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "ExpenseBook"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.expensebook.app"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

install(TARGETS ExpenseBook
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin)
