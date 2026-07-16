#include "ExpenseController.h"
#include "services/AuditService.h"
#include "services/UndoService.h"
#include "utils/Money.h"

namespace eb {

ExpenseController::ExpenseController(QSqlDatabase db, AuditService &audit,
                                     UndoService &undo, QObject *parent)
    : QObject(parent), m_repo(db), m_attachments(db), m_audit(audit), m_undo(undo)
{
}

bool ExpenseController::validate(const Expense &e)
{
    if (!e.date.isValid())        { m_error = "Please choose a valid date."; return false; }
    if (e.amountCents <= 0)       { m_error = "Amount must be greater than zero."; return false; }
    if (e.categoryId <= 0)        { m_error = "Please select a category."; return false; }
    if (e.description.trimmed().isEmpty())
                                  { m_error = "Description is required."; return false; }
    return true;
}

qint64 ExpenseController::save(Expense e)
{
    if (!validate(e))
        return 0;
    if (e.id == 0) {
        e.id = m_repo.insert(e);
        if (!e.id) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Expense", e.id, "insert", money::format(e.amountCents));
    } else {
        if (!m_repo.update(e)) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Expense", e.id, "update", money::format(e.amountCents));
    }
    emit dataChanged();
    return e.id;
}

bool ExpenseController::remove(qint64 id)
{
    auto rec = m_repo.byId(id);
    if (!rec) { m_error = "Record not found."; return false; }
    if (!m_repo.remove(id)) { m_error = m_repo.lastError(); return false; }
    m_undo.pushDeleted(*rec);
    m_audit.log("Expense", id, "delete", rec->description);
    emit dataChanged();
    return true;
}

qint64 ExpenseController::duplicate(qint64 id)
{
    auto rec = m_repo.byId(id);
    if (!rec) { m_error = "Record not found."; return 0; }
    Expense copy = *rec;
    copy.id = 0;
    copy.date = QDate::currentDate();
    copy.referenceNo.clear();
    return save(copy);
}

bool ExpenseController::undoDelete()
{
    if (!m_undo.canUndo())
        return false;
    auto d = m_undo.pop();
    if (auto *e = std::get_if<Expense>(&d)) {
        if (!m_repo.restore(*e)) { m_error = m_repo.lastError(); return false; }
        m_audit.log("Expense", e->id, "restore", e->description);
        emit dataChanged();
        return true;
    }
    // Not an expense — put it back for the income controller to handle.
    m_undo.pushDeleted(std::move(d));
    return false;
}

qint64 ExpenseController::addAttachment(const Attachment &a)
{
    const qint64 id = m_attachments.insert(a);
    if (!id) { m_error = m_attachments.lastError(); return 0; }
    m_audit.log("Attachment", id, "insert", a.fileName);
    return id;
}

bool ExpenseController::removeAttachment(qint64 id)
{
    if (!m_attachments.remove(id)) { m_error = m_attachments.lastError(); return false; }
    m_audit.log("Attachment", id, "delete");
    return true;
}

} // namespace eb
