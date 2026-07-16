-- ExpenseBook schema, version 1.
-- Statements are separated by ';' and executed inside one transaction.

CREATE TABLE IF NOT EXISTS Employees (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    designation TEXT DEFAULT '',
    phone       TEXT DEFAULT '',
    email       TEXT DEFAULT '',
    active      INTEGER NOT NULL DEFAULT 1,
    created_at  TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS Categories (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,
    kind        TEXT NOT NULL DEFAULT 'expense',  -- 'expense' | 'income' | 'both'
    created_at  TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS SubCategories (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    category_id INTEGER NOT NULL REFERENCES Categories(id) ON DELETE CASCADE,
    name        TEXT NOT NULL,
    created_at  TEXT NOT NULL DEFAULT (datetime('now')),
    UNIQUE(category_id, name)
);

CREATE TABLE IF NOT EXISTS PaymentMethods (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS Expenses (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    date              TEXT NOT NULL,               -- ISO yyyy-MM-dd
    employee_id       INTEGER REFERENCES Employees(id) ON DELETE SET NULL,
    description       TEXT NOT NULL DEFAULT '',
    category_id       INTEGER REFERENCES Categories(id) ON DELETE SET NULL,
    subcategory_id    INTEGER REFERENCES SubCategories(id) ON DELETE SET NULL,
    amount            INTEGER NOT NULL,            -- stored in cents to avoid FP drift
    payment_method_id INTEGER REFERENCES PaymentMethods(id) ON DELETE SET NULL,
    reference_no      TEXT DEFAULT '',
    remarks           TEXT DEFAULT '',
    created_at        TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at        TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_expenses_date     ON Expenses(date);
CREATE INDEX IF NOT EXISTS idx_expenses_category ON Expenses(category_id);
CREATE INDEX IF NOT EXISTS idx_expenses_employee ON Expenses(employee_id);

CREATE TABLE IF NOT EXISTS Income (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    date              TEXT NOT NULL,
    source            TEXT NOT NULL DEFAULT '',
    description       TEXT NOT NULL DEFAULT '',
    category_id       INTEGER REFERENCES Categories(id) ON DELETE SET NULL,
    amount            INTEGER NOT NULL,
    payment_method_id INTEGER REFERENCES PaymentMethods(id) ON DELETE SET NULL,
    reference_no      TEXT DEFAULT '',
    remarks           TEXT DEFAULT '',
    created_at        TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX IF NOT EXISTS idx_income_date ON Income(date);

CREATE TABLE IF NOT EXISTS Attachments (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    expense_id INTEGER NOT NULL REFERENCES Expenses(id) ON DELETE CASCADE,
    file_name  TEXT NOT NULL,
    mime_type  TEXT DEFAULT '',
    data       BLOB NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS AuditLogs (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    entity     TEXT NOT NULL,
    entity_id  INTEGER NOT NULL,
    action     TEXT NOT NULL,                     -- insert | update | delete | restore
    details    TEXT DEFAULT '',
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS Backups (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    file_path  TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

INSERT OR IGNORE INTO PaymentMethods(name) VALUES
    ('Cash'), ('Bank Transfer'), ('Credit Card'), ('Debit Card'), ('Cheque'), ('UPI / Mobile');

INSERT OR IGNORE INTO Categories(name, kind) VALUES
    ('Fuel','expense'), ('Travel','expense'), ('Office','expense'), ('Maintenance','expense'),
    ('Salary','expense'), ('Utilities','expense'), ('Marketing','expense'), ('Repair','expense'),
    ('Food','expense'), ('Medical','expense'), ('Miscellaneous','both'), ('Sales','income'),
    ('Services','income');
