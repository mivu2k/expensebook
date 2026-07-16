#include "IncomeController.h"
#include "services/AuditService.h"
#include "services/UndoService.h"
#include "utils/Money.h"

namespace eb {

IncomeController::IncomeController(QSqlDatabase db, AuditService &audit,
                                   UndoService &undo, QObject *parent)
    : QObject(parent), m_repo(db), m_audit(audit), m_undo(undo)
{
}

qint64 IncomeController::save(Income i)
{
    if (!i.date.isValid())  { m_error = "Please choose a valid date."; return 0; }
    if (i.amountCents <= 0) { m_error = "Amount must be greater than zero."; return 0; }
    if (i.source.trimmed().isEmpty() && i.description.trimmed().isEmpty()) {
        m_error = "Provide a source or description.";
        return 0;
    }
    if (i.id == 0) {
        i.id = m_repo.insert(i);
        if (!i.id) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Income", i.id, "insert", money::format(i.amountCents));
    } else {
        if (!m_repo.update(i)) { m_error = m_repo.lastError(); return 0; }
        m_audit.log("Income", i.id, "update", money::format(i.amountCents));
    }
    emit dataChanged();
    return i.id;
}

bool IncomeController::remove(qint64 id)
{
    auto rec = m_repo.byId(id);
    if (!rec) { m_error = "Record not found."; return false; }
    if (!m_repo.remove(id)) { m_error = m_repo.lastError(); return false; }
    m_undo.pushDeleted(*rec);
    m_audit.log("Income", id, "delete", rec->description);
    emit dataChanged();
    return true;
}

bool IncomeController::undoDelete()
{
    if (!m_undo.canUndo())
        return false;
    auto d = m_undo.pop();
    if (auto *i = std::get_if<Income>(&d)) {
        if (!m_repo.restore(*i)) { m_error = m_repo.lastError(); return false; }
        m_audit.log("Income", i->id, "restore", i->description);
        emit dataChanged();
        return true;
    }
    m_undo.pushDeleted(std::move(d));
    return false;
}

} // namespace eb
