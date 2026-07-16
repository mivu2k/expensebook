#pragma once
#include "models/Models.h"
#include "repositories/IncomeRepository.h"
#include <QObject>

namespace eb {

class AuditService;
class UndoService;

class IncomeController : public QObject {
    Q_OBJECT
public:
    IncomeController(QSqlDatabase db, AuditService &audit, UndoService &undo,
                     QObject *parent = nullptr);

    QVector<Income> list(const TransactionFilter &f) const { return m_repo.query(f); }
    std::optional<Income> get(qint64 id) const { return m_repo.byId(id); }
    qint64 save(Income i);
    bool remove(qint64 id);
    bool undoDelete();
    QString errorMessage() const { return m_error; }

signals:
    void dataChanged();

private:
    IncomeRepository m_repo;
    AuditService &m_audit;
    UndoService &m_undo;
    QString m_error;
};

} // namespace eb
