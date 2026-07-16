#include "AttachmentRepository.h"
#include <QSqlError>
#include <QSqlQuery>

namespace eb {

QVector<Attachment> AttachmentRepository::byExpense(qint64 expenseId, bool withData) const
{
    QVector<Attachment> out;
    QSqlQuery q(m_db);
    q.prepare(QString("SELECT id, expense_id, file_name, mime_type%1 FROM Attachments "
                      "WHERE expense_id = ? ORDER BY id")
                  .arg(withData ? ", data" : ""));
    q.addBindValue(expenseId);
    if (!q.exec()) { m_lastError = q.lastError().text(); return out; }
    while (q.next()) {
        Attachment a;
        a.id = q.value(0).toLongLong();
        a.expenseId = q.value(1).toLongLong();
        a.fileName = q.value(2).toString();
        a.mimeType = q.value(3).toString();
        if (withData) a.data = q.value(4).toByteArray();
        out.append(a);
    }
    return out;
}

QByteArray AttachmentRepository::dataOf(qint64 id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT data FROM Attachments WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) return q.value(0).toByteArray();
    m_lastError = q.lastError().text();
    return {};
}

qint64 AttachmentRepository::insert(const Attachment &a)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO Attachments(expense_id, file_name, mime_type, data) "
              "VALUES(?,?,?,?)");
    q.addBindValue(a.expenseId);
    q.addBindValue(a.fileName);
    q.addBindValue(a.mimeType);
    q.addBindValue(a.data);
    if (!q.exec()) { m_lastError = q.lastError().text(); return 0; }
    return q.lastInsertId().toLongLong();
}

bool AttachmentRepository::remove(qint64 id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM Attachments WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

} // namespace eb
