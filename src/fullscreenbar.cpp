#include <QApplication>
#include <QDesktopWidget>

#include "fullscreenbar.h"

const int HEIGHT = 100;

FullScreenBar::FullScreenBar(QWidget* parent) : KToolBar(parent, "fullscreenbar")
{
    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());
    int screenHeight = screen.height();
    int screenWidth = screen.width();

    setObjectName("fullscreenbar");
    setAutoFillBackground(true);
    setFixedHeight(HEIGHT);
    setFixedWidth(screenWidth);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    move(0, screenHeight - HEIGHT);
}

FullScreenBar::~FullScreenBar()
{
}

