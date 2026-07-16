# ExpenseBook

Professional desktop Expense & Accounting application.
Modern C++20 · Qt 6 Widgets · SQLite (single `.db` file) · CMake.

## Features

- Dashboard with KPI cards and four charts (monthly expenses, category pie,
  yearly trend, cash flow) — theme-aware, no extra chart dependency required
- Expense entry (Save, Save & New, Clear, Delete, Duplicate) with drag-and-drop
  file/image attachments and image preview (receipt scanner placeholder included)
- Income entry, unlimited categories and sub-categories, employees, payment methods
- Reports: daily / weekly / monthly / quarterly / yearly / custom range with
  filters for employee, category, sub-category, payment method, amount range
  and description text
- Export to CSV, Excel (`.xlsx` via QXlsx) and PDF; ledger-style printing
- Global instant search (Ctrl+F), sortable tables everywhere
- Undo delete (Ctrl+Z), audit log, automatic backup on exit,
  manual backup/restore with history
- Light and dark Office-style themes, configurable currency, company name,
  logo, backup path and default printer

## Folder structure

```
ExpenseBook/
├── CMakeLists.txt
├── resources/
│   ├── resources.qrc
│   ├── sql/schema.sql          # schema + seed data (migration v1)
│   └── styles/{light,dark}.qss
└── src/
    ├── main.cpp
    ├── core/          Database, MigrationManager, AppSettings
    ├── models/        Models.h (plain value types; money in integer cents)
    ├── repositories/  one repository per table (SQL lives only here)
    ├── services/      Audit, Undo, Backup, Export, Report
    ├── controllers/   Expense, Income, Category, Employee, Report
    ├── ui/            MainWindow, AppContext (composition root)
    │   ├── pages/     one QWidget per navigation page
    │   ├── dialogs/   EmployeeDialog, CategoryDialog
    │   └── widgets/   SimpleCharts, AttachmentListWidget
    └── utils/         Money, Theme
```

Architecture: Clean Architecture layering (UI → controllers → repositories/services
→ SQLite), MVC per page, repository pattern for all persistence, dependency
injection through `AppContext`.

## Building

Requirements: CMake ≥ 3.21, a C++20 compiler, Qt 6.4+ (Widgets, Sql, PrintSupport).
Qt Charts and QXlsx are optional; QXlsx is fetched automatically at configure
time (`-DEB_WITH_XLSX=OFF` to disable, which removes only the Excel export).

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/<platform>
cmake --build build -j
```

The database is created automatically on first launch at the platform data
directory (e.g. `~/Library/Application Support/ExpenseBook/ExpenseBook/expensebook.db`
on macOS, `%APPDATA%/ExpenseBook/ExpenseBook/` on Windows). Schema migrations
run automatically and are versioned with `PRAGMA user_version`
(see `src/core/MigrationManager.cpp` — append new migrations to the list).

## Packaging

### Windows (.exe installer)
```bat
cmake --build build --config Release
windeployqt build\Release\ExpenseBook.exe
```
Then wrap the folder with NSIS or Inno Setup, or use CPack (`cpack -G NSIS`).

### macOS (.dmg)
```sh
cmake --build build
macdeployqt build/ExpenseBook.app -dmg
```
For Intel Macs build with `-DCMAKE_OSX_ARCHITECTURES=x86_64`
(or `"x86_64;arm64"` for a universal binary). Sign/notarize before distribution.

### Linux (AppImage)
```sh
cmake --install build --prefix AppDir/usr
# linuxdeploy + Qt plugin: https://github.com/linuxdeploy/linuxdeploy
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage
```

## Keyboard shortcuts

| Shortcut | Action |
|---|---|
| Ctrl+1 … Ctrl+9 | Jump to page |
| Ctrl+N | New expense |
| Ctrl+S | Save current expense form |
| Ctrl+F | Focus global search |
| Ctrl+Z | Undo last delete |
