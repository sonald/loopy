#ifndef INFOLABEL_H
#define INFOLABEL_H

#include <QLabel>
#include <QContextMenuEvent>
#include <QMouseEvent>

class MainWindow;

class InfoLabel : public QLabel
{
    Q_OBJECT

public:
    InfoLabel(MainWindow* parent);
    ~InfoLabel();

private:
    MainWindow* parent;

private slots:
    void contextMenuEvent(QContextMenuEvent *e);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
};
#endif // INFOLABEL_H
