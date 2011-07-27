#include <QApplication>
#include <QDesktopWidget>

#include "fullscreenplaylist.h"

const int WIDTH = 300;

FullScreenPlaylist::FullScreenPlaylist(QWidget* parent) : KToolBar(parent, "fullscreenplaylist")
{
    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());
    int screenHeight = screen.height();
    int screenWidth = screen.width();

    setObjectName("fullscreenplaylist");
    setHidden(true);
    setAutoFillBackground(true);
    setFixedHeight(screenHeight);
    setFixedWidth(WIDTH);
    move(screenWidth - WIDTH, 0 );
}

FullScreenPlaylist::~FullScreenPlaylist()
{
}
