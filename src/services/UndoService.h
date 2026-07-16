#pragma once
#include "models/Models.h"
#include <QObject>
#include <QStack>
#include <variant>

namespace eb {

/// Keeps a stack of deleted records so the user can undo deletions
/// (Ctrl+Z). Records live in memory for the session.
class UndoService : public QObject {
    Q_OBJECT
public:
    using Deleted = std::variant<Expense, Income>;

    void pushDeleted(Deleted d);
    bool canUndo() const { return !m_stack.isEmpty(); }
    /// Pops the most recent deletion. Caller restores it via the
    /// matching repository.
    Deleted pop();

signals:
    void availabilityChanged(bool canUndo);

private:
    QStack<Deleted> m_stack;
};

} // namespace eb
