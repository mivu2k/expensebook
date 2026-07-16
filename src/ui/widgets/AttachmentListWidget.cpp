#include "AttachmentListWidget.h"

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMimeData>
#include <QMimeDatabase>

namespace eb {

AttachmentListWidget::AttachmentListWidget(QWidget *parent) : QListWidget(parent)
{
    setAcceptDrops(true);
    setToolTip("Drop receipt files or images here, or right-click to add.");
    setMaximumHeight(90);
    connect(this, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it) {
        emit previewRequested(it->data(Qt::UserRole).toLongLong(), it->text());
    });
}

void AttachmentListWidget::setExisting(const QVector<Attachment> &existing)
{
    clearAll();
    for (const Attachment &a : existing) {
        auto *it = new QListWidgetItem("📎 " + a.fileName, this);
        it->setData(Qt::UserRole, a.id);
    }
}

void AttachmentListWidget::clearAll()
{
    clear();
    m_staged.clear();
    m_removed.clear();
}

void AttachmentListWidget::addFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return;
    Attachment a;
    a.fileName = QFileInfo(path).fileName();
    a.mimeType = QMimeDatabase().mimeTypeForFile(path).name();
    a.data = f.readAll();
    m_staged.append(a);
    auto *it = new QListWidgetItem("🆕 " + a.fileName, this);
    it->setData(Qt::UserRole, 0); // staged, no id yet
    it->setData(Qt::UserRole + 1, m_staged.size() - 1);
}

void AttachmentListWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void AttachmentListWidget::dragMoveEvent(QDragMoveEvent *e)
{
    e->acceptProposedAction();
}

void AttachmentListWidget::dropEvent(QDropEvent *e)
{
    for (const QUrl &u : e->mimeData()->urls())
        if (u.isLocalFile())
            addFile(u.toLocalFile());
    e->acceptProposedAction();
}

void AttachmentListWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    QAction *add = menu.addAction("Add file…");
    QAction *scan = menu.addAction("Scan receipt… (coming soon)");
    QAction *del = nullptr;
    if (currentItem())
        del = menu.addAction("Remove selected");
    QAction *chosen = menu.exec(e->globalPos());
    if (chosen == add) {
        const QStringList files = QFileDialog::getOpenFileNames(
            this, "Add attachments", QString(),
            "Images & documents (*.png *.jpg *.jpeg *.gif *.pdf);;All files (*)");
        for (const QString &f : files)
            addFile(f);
    } else if (chosen == scan) {
        // Receipt scanner integration point (placeholder by design).
    } else if (del && chosen == del) {
        QListWidgetItem *it = currentItem();
        const qint64 id = it->data(Qt::UserRole).toLongLong();
        if (id > 0) {
            m_removed.append(id);
        } else {
            const int idx = it->data(Qt::UserRole + 1).toInt();
            if (idx >= 0 && idx < m_staged.size())
                m_staged[idx].fileName.clear(); // tombstone; skipped on save
        }
        delete it;
    }
}

} // namespace eb
