#pragma once
#include "models/Models.h"
#include "repositories/AttachmentRepository.h"
#include "repositories/ExpenseRepository.h"
#include <QObject>

namespace eb {

class AuditService;
class UndoService;

/// Mediates between the Expense UI and the persistence layer, adding
/// validation, audit logging and undo support.
class ExpenseController : public QObject {
    Q_OBJECT
public:
    ExpenseController(QSqlDatabase db, AuditService &audit, UndoService &undo,
                      QObject *parent = nullptr);

    QVector<Expense> list(const TransactionFilter &f) const { return m_repo.query(f); }
    std::optional<Expense> get(qint64 id) const { return m_repo.byId(id); }

    /// Validates and saves; inserts when e.id == 0, updates otherwise.
    /// Returns the record id, or 0 with errorMessage() set.
    qint64 save(Expense e);
    bool remove(qint64 id);           // pushes onto the undo stack
    qint64 duplicate(qint64 id);      // copies the record with today's date
    bool undoDelete();

    // Attachments
    QVector<Attachment> attachments(qint64 expenseId) const
    { return m_attachments.byExpense(expenseId); }
    qint64 addAttachment(const Attachment &a);
    bool removeAttachment(qint64 id);
    QByteArray attachmentData(qint64 id) const { return m_attachments.dataOf(id); }

    QString errorMessage() const { return m_error; }

signals:
    void dataChanged();

private:
    bool validate(const Expense &e);

    ExpenseRepository m_repo;
    AttachmentRepository m_attachments;
    AuditService &m_audit;
    UndoService &m_undo;
    QString m_error;
};

} // namespace eb
