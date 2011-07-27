#ifndef FULLSCREENBAR_H
#define FULLSCREENBAR_H

#include <KToolBar>

class FullScreenBar : public KToolBar
{
    Q_OBJECT

public:
    FullScreenBar(QWidget* parent);
    ~FullScreenBar();
};

#endif // FULLSCREENBAR_H
