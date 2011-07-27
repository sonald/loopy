#ifndef FULLSCREENPLAYLIST_H
#define FULLSCREENPLAYLIST_H
#include <KToolBar>

class FullScreenPlaylist : public KToolBar
{
    Q_OBJECT

public:
    FullScreenPlaylist(QWidget* parent);
    ~FullScreenPlaylist();
};
#endif // FULLSCREENPLAYLIST_H
