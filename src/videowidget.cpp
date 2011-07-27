#include <kxmlguifactory.h>

#include <QBitmap>

#include "videowidget.h"
#include "mainwindow.h"
#include "fullscreenbar.h"
#include "fullscreenplaylist.h"
#include "settings.h"

const int HIDE_CURSOR_TIME = 1000;
const int MOUSE_REGION = 20;

VideoWidget::VideoWidget(MainWindow *parent) : Phonon::VideoWidget(parent)
{
    this->parent = parent;
    setAcceptDrops(true);

    fullScreenBar = new FullScreenBar(this);
    fullScreenPlaylist = new FullScreenPlaylist(this);

    hideCursorTimer = new QTimer(this);
    hideCursorTimer->setInterval(HIDE_CURSOR_TIME);
    hideCursorTimer->setSingleShot(true);
    connect(hideCursorTimer, SIGNAL(timeout()), SLOT(hideCursor()));
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = static_cast<QMenu*>(parent->guiFactory()->container("video", parent));
    popup->popup(e->globalPos());
}

void VideoWidget::wheelEvent(QWheelEvent * e)
{
    if (e->delta() > 0)
        parent->mouseWheelSeekBackward();
    else
        parent->mouseWheelSeekForward();

    e->accept();
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && childrenRect().y() == y()) {
        parent->toggleFullscreen();
        e->accept();
    }
}

void VideoWidget::mouseMoveEvent(QMouseEvent * e)
{
    unsetCursor();
    hideCursorTimer->start();

    if (parent->isFullScreen()) {
        int y = e->pos().y();
        int x = e->pos().x();

        if (y >= height() - MOUSE_REGION) {
            fullScreenBar->show();
            fullScreenPlaylist->hide();
            hideCursorTimer->stop();
            return;
        }
        if (y <= height() - MOUSE_REGION) {
            fullScreenBar->hide();
        }
        if (x >= width() - MOUSE_REGION) {
            fullScreenPlaylist->show();
            fullScreenBar->hide();
            hideCursorTimer->stop();
            return;
        }
        if (x <= width() - 1) {
            fullScreenPlaylist->hide();
        }
    }
}

void VideoWidget::hideCursor()
{
    //Workaround for Xine backend KDE 4.3
    QBitmap empty(QSize(32, 32));
    empty.clear();
    QCursor blankCursor(empty, empty);
    setCursor(blankCursor);

    //setCursor(Qt::BlankCursor);
}

void VideoWidget::dropEvent(QDropEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    if (!uriList.isEmpty()) {
        parent->openUrls(uriList);
    }
}

void VideoWidget::dragEnterEvent(QDragEnterEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    e->setAccepted(!uriList.isEmpty());
}
