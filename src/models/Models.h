#pragma once
#include <QByteArray>
#include <QDate>
#include <QString>

namespace eb {

// Domain models are plain value types. Money is stored as integer cents
// (qint64) end-to-end; see utils/Money.h for formatting/parsing.

struct Employee {
    qint64 id = 0;
    QString name;
    QString designation;
    QString phone;
    QString email;
    bool active = true;
};

struct Category {
    qint64 id = 0;
    QString name;
    QString kind = "expense"; // expense | income | both
};

struct SubCategory {
    qint64 id = 0;
    qint64 categoryId = 0;
    QString name;
};

struct PaymentMethod {
    qint64 id = 0;
    QString name;
};

struct Expense {
    qint64 id = 0;
    QDate date;
    qint64 employeeId = 0;      // 0 = none
    QString description;
    qint64 categoryId = 0;
    qint64 subCategoryId = 0;
    qint64 amountCents = 0;
    qint64 paymentMethodId = 0;
    QString referenceNo;
    QString remarks;
    // Denormalized display names filled in by list queries.
    QString employeeName, categoryName, subCategoryName, paymentMethodName;
};

struct Income {
    qint64 id = 0;
    QDate date;
    QString source;
    QString description;
    qint64 categoryId = 0;
    qint64 amountCents = 0;
    qint64 paymentMethodId = 0;
    QString referenceNo;
    QString remarks;
    QString categoryName, paymentMethodName;
};

struct Attachment {
    qint64 id = 0;
    qint64 expenseId = 0;
    QString fileName;
    QString mimeType;
    QByteArray data;
};

struct AuditLog {
    qint64 id = 0;
    QString entity;
    qint64 entityId = 0;
    QString action;
    QString details;
    QString createdAt;
};

/// Shared filter for expense/income queries and reports.
struct TransactionFilter {
    QDate from, to;              // invalid = unbounded
    qint64 employeeId = 0;       // 0 = any
    qint64 categoryId = 0;
    qint64 subCategoryId = 0;
    qint64 paymentMethodId = 0;
    qint64 minCents = -1, maxCents = -1;  // -1 = unbounded
    QString text;                // matches description / reference / remarks
};

} // namespace eb
