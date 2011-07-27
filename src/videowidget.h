#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QTimer>

#include <Phonon/VideoWidget>

class MainWindow;
class FullScreenBar;
class FullScreenPlaylist;

class VideoWidget : public Phonon::VideoWidget
{
    Q_OBJECT

public:
    VideoWidget(MainWindow* parent);
    ~VideoWidget();
    FullScreenBar *fullScreenBar;
    FullScreenPlaylist *fullScreenPlaylist;

private:
    QTimer *hideCursorTimer;
    MainWindow* parent;

private slots:
    void hideCursor();
    void contextMenuEvent(QContextMenuEvent *e);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
};

#endif // VIDEOWIDGET_H
