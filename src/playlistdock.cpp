#include <QAction>
#include <QDebug>
#include <QAbstractItemView>
#include <QVBoxLayout>

#include <KIcon>
#include <KLocale>

#include "playlistdock.h"
#include "mainwindow.h"

PlaylistDock::PlaylistDock(MainWindow* parent)
{
    this->parent = parent;
    setObjectName("playlistdock");
    setWindowTitle(i18n("Playlist"));
    setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    widget = new QWidget;
    widget->setLayout(layout);

    visiblePlayList = new KListWidget();
    visiblePlayList->setAlternatingRowColors(true);
    visiblePlayList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    visiblePlayList->setObjectName("playlist");
    connect(visiblePlayList, SIGNAL(executed(QListWidgetItem *)), this, SLOT(playItem()));

    setupActions();
    layout->addWidget(visiblePlayList);
    layout->addWidget(toolbar);

    setWidget(widget);
}

PlaylistDock::~PlaylistDock()
{
}

void PlaylistDock::setupActions()
{
    QAction *deleteAllAction = new QAction(this);
    deleteAllAction->setText(i18n("Clear playlist"));
    deleteAllAction->setIcon(KIcon("edit-clear-list"));
    connect(deleteAllAction, SIGNAL(triggered()), this, SLOT(deleteAll()));

    QAction *addItemAction = new QAction(this);
    addItemAction->setText(i18n("Add file(s)"));
    addItemAction->setIcon(KIcon("list-add"));
    connect(addItemAction, SIGNAL(triggered()), parent, SLOT(addFile()));

    QAction *deleteItemAction = new QAction(this);
    deleteItemAction->setText(i18n("Clear selected"));
    deleteItemAction->setIcon(KIcon("list-remove"));
    connect(deleteItemAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    QAction* moveItemUpAction = new QAction(this);
    moveItemUpAction->setText(i18n("Move up"));
    moveItemUpAction->setIcon(KIcon("arrow-up"));
    connect(moveItemUpAction, SIGNAL(triggered()), this, SLOT(moveItemUp()));

    QAction* moveItemDownAction = new QAction(this);
    moveItemDownAction->setText(i18n("Move down"));
    moveItemDownAction->setIcon(KIcon("arrow-down"));
    connect(moveItemDownAction, SIGNAL(triggered()), this, SLOT(moveItemDown()));

    QAction* skipForwardAction = new QAction(this);
    skipForwardAction->setText(i18n("Playlist Skip Forward"));
    skipForwardAction->setIcon(KIcon("media-skip-forward"));
    connect(skipForwardAction, SIGNAL(triggered()), parent, SLOT(skipForward()));

    QAction* skipBackwardAction = new QAction(this);
    skipBackwardAction->setText(i18n("Playlist Skip Backward"));
    skipBackwardAction->setIcon(KIcon("media-skip-backward"));
    connect(skipBackwardAction, SIGNAL(triggered()), parent, SLOT(skipBackward()));

    toolbar = new QToolBar(this);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setObjectName("playlisttoolbar");
    toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    toolbar->addAction(deleteAllAction);
    toolbar->addAction(addItemAction);
    toolbar->addAction(deleteItemAction);
    toolbar->addAction(moveItemUpAction);
    toolbar->addAction(moveItemDownAction);
    toolbar->addAction(skipBackwardAction);
    toolbar->addAction(skipForwardAction);
}

void PlaylistDock::playItem()
{
    int rowNumber = visiblePlayList->currentRow();
    bool wasPlaying = parent->mediaObject->state() == Phonon::PlayingState;

    parent->mediaObject->stop();
    parent->mediaObject->clearQueue();

    if (rowNumber >= parent->hiddenPlayList.size()) {
        return;
    }

    parent->mediaObject->setCurrentSource(parent->hiddenPlayList[rowNumber]);

    if (wasPlaying) {
        parent->mediaObject->play();
    }
    else {
        parent->mediaObject->stop();
    }
}

void PlaylistDock::deleteAll()
{
    visiblePlayList->clear();
    parent->hiddenPlayList.clear();
}

void PlaylistDock::deleteItem()
{
    QListWidgetItem *item = visiblePlayList->currentItem();

    if (item) {
        int r = visiblePlayList->row(item);
        parent->hiddenPlayList.removeAt(r);
        visiblePlayList->takeItem(r);
        delete item;
    }
}

void PlaylistDock::moveItemUp()
{
    int row = visiblePlayList->currentRow();
    if (row > 0) {
        QListWidgetItem* item = visiblePlayList->takeItem(row);
        visiblePlayList->insertItem(row - 1, item);
        visiblePlayList->setCurrentItem(item);
        parent->hiddenPlayList.move(row - 1, row);
    }
}

void PlaylistDock::moveItemDown()
{
    int row = visiblePlayList->currentRow();
    if (row < visiblePlayList->count() - 1) {
        QListWidgetItem* item = visiblePlayList->takeItem(row);
        visiblePlayList->insertItem(row + 1, item);
        visiblePlayList->setCurrentItem(item);
        parent->hiddenPlayList.move(row + 1, row);
    }
}

void PlaylistDock::dropEvent(QDropEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    if (!uriList.isEmpty()) {
        parent->addUrls(uriList);
    }
}

void PlaylistDock::dragEnterEvent(QDragEnterEvent *e)
{
    KUrl::List uriList = KUrl::List::fromMimeData(e->mimeData());
    e->setAccepted(!uriList.isEmpty());
}
