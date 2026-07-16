#pragma once
#include "models/Models.h"
#include <QListWidget>

namespace eb {

/// Drag-and-drop attachment list for the expense form. Files dropped
/// (or added via the context menu) are staged in memory and persisted
/// by the page when the expense is saved. Double-click previews images.
class AttachmentListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit AttachmentListWidget(QWidget *parent = nullptr);

    void setExisting(const QVector<Attachment> &existing);
    QVector<Attachment> stagedNew() const { return m_staged; }
    QVector<qint64> removedIds() const { return m_removed; }
    void clearAll();

signals:
    void previewRequested(qint64 attachmentId, const QString &fileName);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    void addFile(const QString &path);

    QVector<Attachment> m_staged;   // not yet in the database
    QVector<qint64> m_removed;      // existing rows the user removed
};

} // namespace eb
