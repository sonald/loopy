#include <KIcon>
#include <kxmlguifactory.h>

#include "infolabel.h"
#include "mainwindow.h"

InfoLabel::InfoLabel(MainWindow *parent)
{
    this->parent = parent;
    setObjectName("infolabel");
    setAcceptDrops(true);
    setAlignment(Qt::AlignCenter);
    setAutoFillBackground(true);
    setPixmap(KIcon("applications-multimedia").pixmap(128, 128));
}

InfoLabel::~InfoLabel()
{
}

void InfoLabel::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = static_cast<QMenu*>(parent->guiFactory()->container("context_info_popup", parent));
    popup->popup(e->globalPos());
}

void InfoLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        parent->toggleFullscreen();
        e->accept();
    }
}

void InfoLabel::dropEvent(QDropEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    if (!uriList.isEmpty()) {
        parent->openUrls(uriList);
    }
}

void InfoLabel::dragEnterEvent(QDragEnterEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    e->setAccepted(!uriList.isEmpty());
}
