#ifndef PLAYLISTDOCK_H
#define PLAYLISTDOCK_H

#include <KListWidget>

#include <QDockWidget>
#include <QToolBar>

class MainWindow;

class PlaylistDock : public QDockWidget
{
    Q_OBJECT

public:
    PlaylistDock(MainWindow* parent);
    ~PlaylistDock();
    KListWidget *visiblePlayList;
    QToolBar *toolbar;
    QWidget *widget;

public slots:
    void deleteAll();

private:
    MainWindow* parent;

private slots:
    void setupActions();
    void deleteItem();
    void playItem();
    void moveItemDown();
    void moveItemUp();

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

};
#endif // PLAYLISTDOCK_H
