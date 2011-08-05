#include <KApplication>
#include <KActionCollection>
#include <KAction>
#include <KStandardAction>
#include <KToggleAction>
#include <KToggleFullScreenAction>
#include <KConfigGroup>
#include <KStandardDirs>

#include <QDBusInterface>
#include <QVBoxLayout>

#include "mainwindow.h"

static inline bool isPlayable(const Phonon::MediaSource::Type t)
{
    return t != Phonon::MediaSource::Invalid && t != Phonon::MediaSource::Empty;
}


MainWindow::MainWindow() : KXmlGuiWindow()
{
    //Phonon
    mediaObject = new Phonon::MediaObject(this);
    mediaController = new Phonon::MediaController(mediaObject);
    m_videoWidget = new VideoWidget(this);
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(mediaObject, audioOutput);
    Phonon::createPath(mediaObject, m_videoWidget);

    mediaObject->setTickInterval(1000);

    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(hasVideoChanged(bool)),
            this, SLOT(hasVideoChanged(bool)));
    connect(mediaObject, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)),
            this, SLOT(sourceChanged(const Phonon::MediaSource &)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
    connect(mediaObject, SIGNAL(finished()), this, SLOT(finished()));
    connect(mediaObject, SIGNAL(metaDataChanged()), this, SLOT(updateCaption()));//For DVD

    m_infoLabel = new InfoLabel(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_videoWidget);
    m_videoWidget->hide();
    mainLayout->addWidget(m_infoLabel);

    QWidget *widget = new QWidget;
    widget->setLayout(mainLayout);

    setCentralWidget(widget);

    m_playlistDock = new PlaylistDock(this);
    addDockWidget(Qt::RightDockWidgetArea, m_playlistDock);

    m_timeLabel = new TimeLabel(statusBar());
    m_timeLabel->setObjectName("timelabel");
    connect(mediaObject, SIGNAL(tick(qint64)), m_timeLabel, SLOT(setCurrentTime(qint64)));
    connect(mediaObject, SIGNAL(totalTimeChanged(qint64)), m_timeLabel, SLOT(setTotalTime(qint64)));
    statusBar()->addPermanentWidget(m_timeLabel);

    //Eats the tooltips in fullscreenmode
    m_toolTipEater = new ToolTipEater(this);

    setupActions();
    setupFullScreenToolBar();

    setupGUI(); //load loopyui.rc file
    {
        //qDebug()<<actionCollection()->actions();

        //Disable Help menu actions
        QAction *helpContentsAction = actionCollection()->action("help_contents");
        actionCollection()->removeAction(helpContentsAction);

        QAction *helpWhatsThisAction = actionCollection()->action("help_whats_this");
        actionCollection()->removeAction(helpWhatsThisAction);

        QAction *optionsShowStatusbar = actionCollection()->action("options_show_statusbar");
        actionCollection()->removeAction(optionsShowStatusbar);

        setStandardToolBarMenuEnabled (false);
    }

    disableScreenSaverTimer = new QTimer(this);
    connect(disableScreenSaverTimer, SIGNAL(timeout()), this, SLOT(disableScreenSaver()));

    loadStyleSheet(Settings::theme());

    createTrayIcon();

    if (Settings::hidePlaylist()) m_playlistDock->setVisible(false);

    //Restore windowstate after crash, fullscreen exit etc.
    bool wasVisible = KConfigGroup(KGlobal::config(), "GUIOptions")
                      .readEntry("ViewMode", false);

    menuBar()->setHidden(wasVisible);
    toolBar()->setHidden(wasVisible);
    statusBar()->setHidden(wasVisible);

    audioOutput->setMuted(KConfigGroup(KGlobal::config(), "Audio").
                          readEntry("Muted", false));

    audioOutput->setVolume(qreal(KConfigGroup(KGlobal::config(), "Audio").
                                 readEntry("Volume", 1.0)));

    updateTitleMenu();
    updateChapterMenu();
    updateAngleMenu();
}

MainWindow::~MainWindow()
{
    //Save Settings
    actionOpenRecent->saveEntries(KConfigGroup(KGlobal::config(), "Recent Files"));
    KGlobal::config()->group("PlayingOptions").writeEntry("Repeat", action("repeatAction")->isChecked());
    KGlobal::config()->group("GUIOptions").writeEntry("ViewMode", action("toggleViewMode")->isChecked());
    KGlobal::config()->group("Audio").writeEntry("Volume", audioOutput->volume());
    KGlobal::config()->group("Audio").writeEntry("Muted", audioOutput->isMuted());
}

void MainWindow::setupActions()
{
    //File menu
    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    KAction* openUrlAction = actionCollection()->addAction("openUrlAction");
    openUrlAction->setText(i18n("Open URL"));
    openUrlAction->setIcon(KIcon("uri-mms"));
    openUrlAction->setShortcut(Qt::CTRL | Qt::Key_U);
    connect(openUrlAction, SIGNAL(triggered()), this, SLOT(openUrl()));

    actionOpenRecent = KStandardAction::openRecent(this, SLOT(openUrl(KUrl)), actionCollection());
    actionOpenRecent->loadEntries(KConfigGroup(KGlobal::config(), "Recent Files"));

    KAction* playDVDAction = actionCollection()->addAction("playDVDAction");
    playDVDAction->setText(i18n("Play DVD"));
    playDVDAction->setIcon(KIcon("media-optical-dvd"));
    connect(playDVDAction, SIGNAL(triggered()), this, SLOT(playDVD()));

    //KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    //Play menu
    KAction* playPauseAction = actionCollection()->addAction("pause");
    playPauseAction->setText(i18n("Play"));
    playPauseAction->setIcon(KIcon("media-playback-start"));
    playPauseAction->setShortcut(KShortcut(Qt::Key_Space, Qt::Key_MediaPlay));
    connect(playPauseAction, SIGNAL(triggered()), this, SLOT(playPause()));
    playPauseAction->setEnabled(false);

    KAction* seekForwardAction = actionCollection()->addAction("seekforward");
    seekForwardAction->setText(i18n("Seek Forward"));
    seekForwardAction->setShortcut(Qt::Key_Right);
    seekForwardAction->setIcon(KIcon("media-seek-forward"));
    connect(seekForwardAction, SIGNAL(triggered()), this, SLOT(seekForward()));
    seekForwardAction->setEnabled(false);
    
    KAction* jumpForwardAction = actionCollection()->addAction("jumpforward");
    jumpForwardAction->setText(i18n("Jump Forward"));
    jumpForwardAction->setShortcut(Qt::Key_Up);
    jumpForwardAction->setIcon(KIcon("media-skip-forward"));
    connect(jumpForwardAction, SIGNAL(triggered()), this, SLOT(jumpForward()));
    jumpForwardAction->setEnabled(false);

    KAction* seekBackwardAction =  actionCollection()->addAction("seekbackward");
    seekBackwardAction->setText(i18n("Seek Backward"));
    seekBackwardAction->setIcon(KIcon("media-seek-backward"));
    seekBackwardAction->setShortcut(Qt::Key_Left);
    connect(seekBackwardAction, SIGNAL(triggered()), this, SLOT(seekBackward()));
    seekBackwardAction->setEnabled(false);

    KAction* jumpBackwardAction = actionCollection()->addAction("jumpbackward");
    jumpBackwardAction->setText(i18n("Jump Backward"));
    jumpBackwardAction->setShortcut(Qt::Key_Down);
    jumpBackwardAction->setIcon(KIcon("media-skip-backward"));
    connect(jumpBackwardAction, SIGNAL(triggered()), this, SLOT(jumpBackward()));
    jumpBackwardAction->setEnabled(false);

    KAction* skipForwardAction = actionCollection()->addAction("skipforward");
    skipForwardAction->setText(i18n("Playlist Skip Forward"));
    skipForwardAction->setIcon(KIcon("media-skip-forward"));
    skipForwardAction->setShortcut(KShortcut(Qt::Key_PageDown, Qt::Key_MediaNext));
    connect(skipForwardAction, SIGNAL(triggered()), this, SLOT(skipForward()));
    skipForwardAction->setEnabled(false);

    KAction* skipBackwardAction = actionCollection()->addAction("skipbackward");
    skipBackwardAction->setText(i18n("Playlist Skip Backward"));
    skipBackwardAction->setIcon(KIcon("media-skip-backward"));
    skipBackwardAction->setShortcut(KShortcut(Qt::Key_PageUp, Qt::Key_MediaPrevious));
    connect(skipBackwardAction, SIGNAL(triggered()), this, SLOT(skipBackward()));
    skipBackwardAction->setEnabled(false);

    KAction* repeatAction = actionCollection()->addAction("repeatAction");
    repeatAction->setText(i18n("Repeat"));
    repeatAction->setIcon(KIcon("view-refresh"));
    repeatAction->setCheckable(true);
    repeatAction->setChecked(KConfigGroup(KGlobal::config(), "PlayingOptions")
                             .readEntry("Repeat", false));

    //Video menu
    QActionGroup *aspectGroup = new QActionGroup(this);
    connect(aspectGroup, SIGNAL(triggered(QAction *)), this, SLOT(aspectChanged(QAction *)));
    aspectGroup->setExclusive(true);

    QAction* aspectActionAuto = actionCollection()->addAction("aspectActionAuto");
    aspectActionAuto->setText(i18n("Auto"));
    aspectActionAuto->setCheckable(true);
    aspectActionAuto->setChecked(true);
    aspectGroup->addAction(aspectActionAuto);

    QAction* aspectActionScale = actionCollection()->addAction("aspectActionScale");
    aspectActionScale->setText(i18n("Scale"));
    aspectActionScale->setCheckable(true);
    aspectGroup->addAction(aspectActionScale);

    QAction* aspectAction16_9 = actionCollection()->addAction("aspectAction16_9");
    aspectAction16_9->setText(i18n("16/9"));
    aspectAction16_9->setCheckable(true);
    aspectGroup->addAction(aspectAction16_9);

    QAction* aspectAction4_3 = actionCollection()->addAction("aspectAction4_3");
    aspectAction4_3->setText(i18n("4/3"));
    aspectAction4_3->setCheckable(true);
    aspectGroup->addAction(aspectAction4_3);

    QActionGroup *scaleGroup = new QActionGroup(this);
    connect(scaleGroup, SIGNAL(triggered(QAction *)), this, SLOT(scaleChanged(QAction *)));
    scaleGroup->setExclusive(true);

    QAction* scaleActionFit = actionCollection()->addAction("scaleActionFit");
    scaleActionFit->setText(i18n("Fit in view"));
    scaleActionFit->setCheckable(true);
    scaleActionFit->setChecked(true);
    scaleGroup->addAction(scaleActionFit);

    QAction* scaleActionCrop = actionCollection()->addAction("scaleActionCrop");
    scaleActionCrop->setText(i18n("Scale and crop"));
    scaleActionCrop->setCheckable(true);
    scaleGroup->addAction(scaleActionCrop);

    /// Subtitle
    subtitlesGroup = new QActionGroup(this);
    connect(subtitlesGroup, SIGNAL(triggered(QAction *)), this, SLOT(subtitleChanged(QAction *)));
    connect(mediaController, SIGNAL(availableSubtitlesChanged()),
            this, SLOT(updateSubtitlesMenu()));
    
    //Video submenus only for DVD
    titleGroup = new QActionGroup(this);
    titleCount = 0;
    connect(titleGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeTitle(QAction*)));
    connect(mediaController, SIGNAL(availableTitlesChanged(int)),
            this, SLOT(titleCountChanged(int)));
    connect(mediaController, SIGNAL(titleChanged(int)), this, SLOT(updateTitleMenu()));

    chapterGroup = new QActionGroup(this);
    chapterCount = 0;
    connect(chapterGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeChapter(QAction*)));
    connect(mediaController, SIGNAL(availableChaptersChanged(int)),
            this, SLOT(chapterCountChanged(int)));
    connect(mediaController, SIGNAL(chapterChanged(int)), this, SLOT(updateChapterMenu()));

    angleGroup = new QActionGroup(this);
    angleCount = 0;
    connect(angleGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeAngle(QAction*)));
    connect(mediaController, SIGNAL(availableAnglesChanged(int)),
            this, SLOT(angleCountChanged(int)));
    connect(mediaController, SIGNAL(angleChanged(int)), this, SLOT(updateAngleMenu()));

    //Audio menu
    KAction* muteAction = actionCollection()->addAction("mute");
    muteAction->setText(i18n("Mute"));
    muteAction->setIcon(KIcon("audio-volume-medium"));
    muteAction->setShortcut(KShortcut(Qt::Key_M, Qt::Key_VolumeMute));
    connect(muteAction, SIGNAL(triggered(bool)), this, SLOT(toggleMuted()));
    connect(audioOutput, SIGNAL(mutedChanged(bool)), this, SLOT(mutedChanged(bool)));

    KAction* increaseVolumeAction = actionCollection()->addAction("increasevolume");
    increaseVolumeAction->setText(i18n("Increase Volume"));
    increaseVolumeAction->setIcon(KIcon("audio-volume-high"));
    increaseVolumeAction->setShortcut(KShortcut(Qt::Key_Plus, Qt::Key_VolumeUp));
    connect(increaseVolumeAction, SIGNAL(triggered()), this, SLOT(increaseVolume()));

    KAction* decreaseVolumeAction = actionCollection()->addAction("decreasevolume");
    decreaseVolumeAction->setText(i18n("Decrease Volume"));
    decreaseVolumeAction->setIcon(KIcon("audio-volume-low"));
    decreaseVolumeAction->setShortcut(KShortcut(Qt::Key_Minus, Qt::Key_VolumeDown));
    connect(decreaseVolumeAction, SIGNAL(triggered()), this, SLOT(decreaseVolume()));

    /// Audio Channels
    channelGroup = new QActionGroup(this);
    connect(channelGroup, SIGNAL(triggered(QAction *)), this, SLOT(channelChanged(QAction *)));
    channelGroup->setExclusive(true);
    connect( mediaController, SIGNAL(availableAudioChannelsChanged()),
             this, SLOT(updateAudioChannelsMenu()));

    
    //Settings menu
    KAction* playListAction = actionCollection()->addAction("playListAction");
    playListAction->setText(i18n("Playlist"));
    playListAction->setIcon(KIcon("view-media-playlist"));
    playListAction->setShortcut(Qt::Key_F9);
    connect(playListAction, SIGNAL(triggered()), m_playlistDock->toggleViewAction(), SLOT(trigger()));

    KAction* toggleViewModeAction = actionCollection()->addAction("toggleViewMode");
    toggleViewModeAction->setText(i18n("Minimal View"));
    toggleViewModeAction->setShortcut(Qt::Key_F3);
    toggleViewModeAction->setCheckable(true);
    toggleViewModeAction->setChecked(KConfigGroup(KGlobal::config(), "GUIOptions")
                                     .readEntry("ViewMode", false));
    connect(toggleViewModeAction, SIGNAL(triggered()),this, SLOT(toggleViewMode()));

    KToggleFullScreenAction* toggleFullScreen = new KToggleFullScreenAction(this);
    toggleFullScreen->setShortcut(Qt::Key_F);
    actionCollection()->addAction("fullscreen", toggleFullScreen);
    connect(toggleFullScreen, SIGNAL(toggled(bool)), MainWindow::window(), SLOT(fullscreen(bool)));

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    //KStandardAction::showMenubar(this, SLOT(showMenuBar()), actionCollection());

    //Phonon sliders
    Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider;
    volumeSlider->setAudioOutput(audioOutput);
    //volumeSlider->setFixedWidth(80);
    volumeSlider->setMaximumWidth(80);
    //volumeSlider->setMuteVisible(0);
    KAction* volume = actionCollection()->addAction("volumeslider");
    volume->setText(i18n("Volume slider"));
    volume->setDefaultWidget(volumeSlider);

    Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider;
    seekSlider->setMediaObject(mediaObject);
    seekSlider->setIconVisible(0);
    KAction* positionSlider = actionCollection()->addAction("positionslider");
    positionSlider->setText(i18n("Position slider"));
    positionSlider->setDefaultWidget(seekSlider);
    
    KAction* reloadThemeAction = actionCollection()->addAction("reloadThemeAction");
    reloadThemeAction->setText(i18n("Reload Theme (for theme development)"));
    reloadThemeAction->setIcon(KIcon("fill-color"));
    connect(reloadThemeAction, SIGNAL(triggered()), this, SLOT(reloadTheme()));
}

void MainWindow::setupFullScreenToolBar()
{
    QToolButton *fullscreenButton = new QToolButton(m_videoWidget->fullScreenBar);
    fullscreenButton->setIcon(KIcon("view-restore"));
    fullscreenButton->setIconSize(QSize(22, 22));
    connect(fullscreenButton, SIGNAL(pressed()), this, SLOT(toggleFullscreen()));

    playPauseButton = new QToolButton(m_videoWidget->fullScreenBar);
    playPauseButton->setIcon(KIcon("media-playback-start"));
    playPauseButton->setIconSize(QSize(48, 48));
    connect(playPauseButton, SIGNAL(pressed()), this, SLOT(playPause()));

    QToolButton *jumpBackwardButton = new QToolButton(m_videoWidget->fullScreenBar);
    jumpBackwardButton->setIcon(KIcon("media-skip-backward"));
    jumpBackwardButton->setIconSize(QSize(36, 36));
    connect(jumpBackwardButton, SIGNAL(pressed()), this, SLOT(jumpBackward()));

    QToolButton *seekBackwardButton = new QToolButton(m_videoWidget->fullScreenBar);
    seekBackwardButton->setIcon(KIcon("media-seek-backward"));
    seekBackwardButton->setIconSize(QSize(36, 36));
    connect(seekBackwardButton, SIGNAL(pressed()), this, SLOT(seekBackward()));

    QToolButton *seekForwardButton = new QToolButton(m_videoWidget->fullScreenBar);
    seekForwardButton->setIcon(KIcon("media-seek-forward"));
    seekForwardButton->setIconSize(QSize(36, 36));
    connect(seekForwardButton, SIGNAL(pressed()), this, SLOT(seekForward()));

    QToolButton *jumpForwardButton = new QToolButton(m_videoWidget->fullScreenBar);
    jumpForwardButton->setIcon(KIcon("media-skip-forward"));
    jumpForwardButton->setIconSize(QSize(36, 36));
    connect(jumpForwardButton, SIGNAL(pressed()), this, SLOT(jumpForward()));

    muteButton = new QToolButton(m_videoWidget->fullScreenBar);
    //muteButton->setIcon(KIcon("player-volume"));
    muteButton->setIcon(KIcon("audio-volume-medium"));
    muteButton->setIconSize(QSize(22, 22));
    connect(muteButton, SIGNAL(pressed()), this, SLOT(toggleMuted()));

    Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider(m_videoWidget->fullScreenBar);
    seekSlider->setMediaObject(mediaObject);
    seekSlider->setIconVisible(0);

    volumeSlider = new QSlider(m_videoWidget->fullScreenBar);
    volumeSlider->setFocusPolicy(Qt::NoFocus);
    volumeSlider->setOrientation(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setFixedWidth(80);
    volumeSlider->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));
    connect(audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));

    m_timeLabelFullScreenBar = new TimeLabel(m_videoWidget->fullScreenBar);
    m_timeLabelFullScreenBar->setObjectName("fullscreenbarTimeLabel");
    connect(mediaObject, SIGNAL(tick(qint64)), m_timeLabelFullScreenBar, SLOT(setCurrentTime(qint64)));
    connect(mediaObject, SIGNAL(totalTimeChanged(qint64)), m_timeLabelFullScreenBar, SLOT(setTotalTime(qint64)));

    m_titleLabelFullScreenBar = new TitleLabel(m_videoWidget->fullScreenBar);

    QHBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->setSpacing(0);
    sliderLayout->setMargin(0);
    sliderLayout->insertSpacing(0, 15);
    sliderLayout->addWidget(seekSlider);
    sliderLayout->insertSpacing(-1, 15);

    QWidget *sliderBox = new QWidget;
    sliderBox->setLayout(sliderLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);
    buttonLayout->insertSpacing(0, 15);
    buttonLayout->addWidget(fullscreenButton);
    buttonLayout->addWidget(m_titleLabelFullScreenBar);
    buttonLayout->addWidget(jumpBackwardButton);
    buttonLayout->addWidget(seekBackwardButton);
    buttonLayout->addWidget(playPauseButton);
    buttonLayout->addWidget(seekForwardButton);
    buttonLayout->addWidget(jumpForwardButton);
    buttonLayout->addWidget(m_timeLabelFullScreenBar);
    buttonLayout->addWidget(muteButton);
    buttonLayout->addWidget(volumeSlider);
    buttonLayout->insertSpacing(-1, 15);

    QWidget *buttonBox = new QWidget;
    buttonBox->setLayout(buttonLayout);

    QVBoxLayout *toolLayout = new QVBoxLayout;
    toolLayout->setSpacing(0);
    toolLayout->setMargin(10);
    toolLayout->addWidget(buttonBox);
    toolLayout->addWidget(sliderBox);

    QWidget *toolBox = new QWidget;
    toolBox->setObjectName("fullscreenbarlayout");
    toolBox->setLayout(toolLayout);
    m_videoWidget->fullScreenBar->addWidget(toolBox);

    m_videoWidget->fullScreenBar->hide();
}

/////////////////////////////////////////////////////
///Add files
////////////////////////////////////////////////////
KCmdLineOptions MainWindow::cmdLineOptions()
{
    KCmdLineOptions options;
    options.add("dvd", ki18n("Play DVD"));
    options.add("add", ki18n("Add Files or URLs to playlist"));
    options.add("+[file]", ki18n("Open Files or URLs to play"));
    return options;
}

void MainWindow::parseArgs()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("dvd")) {
        playDVD();
        args->clear();
        return;
    }

    if (args->count() > 0) {
        QList<KUrl> urls;

        for (int i = 0; i < args->count(); ++i) {
            KUrl url = args->url(i);

            if (url.isValid()) {
                urls.append(url);
            }
        }
        if (!urls.isEmpty()) {
            if (args->isSet("add")) {
                addUrls(urls);
            } else {
                openUrls(urls);
            }
        }
    }
    args->clear();
}

void MainWindow::openFile()
{
    if (Settings::alwaysEnqueue()) {
        addFile();
        return;
    }

    QStringList mimeTypes = getSupportedMimeTypes();
    QList<KUrl> urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///loopy")
                       , mimeTypes.join(" ")
                       , this
                       , i18n("Select file(s) to play"));

    if (!urls.isEmpty()) {
        openUrls(urls);
    }
}

void MainWindow::addFile()
{
    QStringList mimeTypes = getSupportedMimeTypes();
    QList<KUrl> urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///loopy")
                       , mimeTypes.join(" ")
                       , this
                       , i18n("Add file(s) to playlist"));

    if (!urls.isEmpty()) {
        addUrls(urls);
    }
}

QStringList MainWindow::getSupportedMimeTypes()
{
    QStringList mimeTypes =
        Phonon::BackendCapabilities::availableMimeTypes().filter("video/");

    if (mimeTypes.size() == 0) {
        KMimeType::List mimeList = KMimeType::allMimeTypes();

        Q_FOREACH(KMimeType::Ptr mime, mimeList) {
            if (mime->name().startsWith("video/")) {
                mimeTypes << mime->name();
            }
        }
    }

    qDebug() << "Loopy: " << mimeTypes;
    return mimeTypes;
}

void MainWindow::openUrl()
{
    openUrl(KInputDialog::getText(i18n("Open URL"), i18n("Enter a URL:")));
}

void MainWindow::openUrls(const QList<KUrl> &urls)
{
    if (urls.isEmpty()) {
        return;
    }
    if (Settings::alwaysEnqueue()) {
        addUrls(urls);
        return;
    }
    m_playlistDock->deleteAll();
    mediaObject->clearQueue();
    pushUrls(urls);
    
    mediaObject->setCurrentSource(hiddenPlayList.at(0));
    mediaObject->play();
    qDebug() <<"Loopy: start to play";
}

void MainWindow::addUrls(const QList<KUrl> &urls)
{
    if (urls.isEmpty()) {
        return;
    }
    pushUrls(urls);
    
    if (!hiddenPlayList.isEmpty() && mediaObject->state() != Phonon::PlayingState) {
        mediaObject->setCurrentSource(hiddenPlayList.at(0));
        mediaObject->play();
    }
}

void MainWindow::pushUrls(const QList<KUrl> &urls)
{
    Q_FOREACH(const KUrl &url, urls) {
        pushUrl( url );
    }
}

void MainWindow::pushUrl(const KUrl &url)
{
    QString title;
    if (url.isLocalFile()) {
        title = url.toLocalFile();
        title = (title.right(title.length() - title.lastIndexOf('/') - 1));
    } else {
        title = url.prettyUrl();
    }
        
    QListWidgetItem *titleItem = new QListWidgetItem(title);
    titleItem->setIcon(KIcon("media-playback-start"));
    int currentRow = m_playlistDock->visiblePlayList->count();
    m_playlistDock->visiblePlayList->insertItem(currentRow, titleItem);

    if (url.isLocalFile()) {
        hiddenPlayList.append( Phonon::MediaSource(url.toLocalFile()) );
    } else {
        hiddenPlayList.append( Phonon::MediaSource(url) );
    }
}

void MainWindow::openUrl(const KUrl &url)
{
    if (!url.isValid()) {
        return;
    }

    if (Settings::alwaysEnqueue()) {
        QList<KUrl> urls;
        urls.append(url);
        addUrls(urls);
        return;
    }

    m_playlistDock->deleteAll();
    mediaObject->clearQueue();
    pushUrl( url );
    mediaObject->setCurrentSource(hiddenPlayList.at(0));
    mediaObject->play();
}

void MainWindow::playDVD()
{
    m_playlistDock->deleteAll();
    mediaObject->clearQueue();

    QList<Solid::Device> devices =
        Solid::Device::listFromQuery("OpticalDisc.availableContent & 'VideoDvd'");
    QString deviceName;

    if (!devices.isEmpty()) {
        Solid::Block *block = devices.first().as<Solid::Block>();

        if (block != NULL) {
            deviceName = block->device();
        }
    }

    Phonon::MediaSource source(Phonon::Dvd, deviceName);
    //FIXME ->maybe it's better not to show this at the playlist,
    //instead show the DVD titles ? Overkill ???
    QString title;
    title = "DVD";

    QListWidgetItem *titleItem = new QListWidgetItem(title);
    titleItem->setIcon(KIcon("media-playback-start"));
    int currentRow = m_playlistDock->visiblePlayList->count();
    m_playlistDock->visiblePlayList->insertItem(currentRow, titleItem);

    hiddenPlayList.append(source);

    mediaObject->setCurrentSource(hiddenPlayList.at(0));
    mediaObject->play();

}

/////////////////////////////////////////////////////
///Toggle MenuBar (NOT USED !)
////////////////////////////////////////////////////
void MainWindow::showMenuBar()
{
    menuBar()->setVisible(action("options_show_menubar")->isChecked());
}

/////////////////////////////////////////////////////
///Toggle MinimalMode
////////////////////////////////////////////////////
void MainWindow::toggleViewMode()
{
    if (action("toggleViewMode")->isChecked()) {
        KMessageBox::information(this, i18n("You get back to normal view with the F3 key."), "", "ViewModeMessage");
        menuBar()->setHidden(true);
        toolBar("mainToolBar")->setHidden(true);
        statusBar()->setHidden(true);    
    }
    else {
        menuBar()->setVisible(true);
        toolBar("mainToolBar")->setVisible(true);
        statusBar()->setVisible(true);
    }
}

/////////////////////////////////////////////////////
///Fullscreen
////////////////////////////////////////////////////
void MainWindow::fullscreen(bool isFullScreen)
{
    window()->setWindowState((isFullScreen ? Qt::WindowFullScreen : Qt::WindowNoState));

    if (isFullScreen) {
        menuBar()->setHidden(true);
        toolBar("mainToolBar")->setHidden(true);
        statusBar()->setHidden(true);

        isPlayListDock = m_playlistDock->toggleViewAction()->isChecked();
        m_playlistDock->setHidden(true);

        disableScreenSaverTimer->start(30000);
        m_videoWidget->fullScreenPlaylist->addWidget(m_playlistDock->widget);
        m_playlistDock->toolbar->setIconSize(QSize(22,22));
        kapp->installEventFilter(m_toolTipEater);
        action("toggleViewMode")->setDisabled(true);
    }
    else {
        if (action("toggleViewMode")->isChecked()) {
            menuBar()->setHidden(true);
            toolBar("mainToolBar")->setHidden(true);
            statusBar()->setHidden(true);
        }
        else {
            menuBar()->setVisible(true);
            toolBar("mainToolBar")->setVisible(true);
            statusBar()->setVisible(true);
        }

        m_playlistDock->setVisible(isPlayListDock);

        disableScreenSaverTimer->stop();

        m_playlistDock->setWidget(m_playlistDock->widget);
        m_playlistDock->toolbar->setIconSize(QSize(16, 16));
        kapp->removeEventFilter(m_toolTipEater);
        action("toggleViewMode")->setDisabled(false);
    }
}

void MainWindow::toggleFullscreen()
{
    action("fullscreen")->toggle();
}

/////////////////////////////////////////////////////
///Disable ScreenSaver
////////////////////////////////////////////////////
void MainWindow::disableScreenSaver()
{
    QDBusInterface interface("org.freedesktop.ScreenSaver", "/ScreenSaver");
    interface.call(QDBus::NoBlock, "SimulateUserActivity");
}

/////////////////////////////////////////////////////
///Load Stylesheet
////////////////////////////////////////////////////
void MainWindow::loadStyleSheet(QString themeName)
{
    //kapp->setStyle("Plastique");
    //setWindowFlags(Qt::FramelessWindowHint);

    QString themeDir = KStandardDirs::locate("appdata", "themes/" + themeName + "/");
    if (themeDir.isEmpty()) {
        qDebug() << "Loopy: Theme: Couldn't find theme" << themeName;
        kapp->setStyleSheet("");
        return;
    }
    QString styleSheetPath = themeDir + "style.qss";
    QFile file(styleSheetPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Loopy: Theme: Couldn't open" << styleSheetPath;
        kapp->setStyleSheet("");
        return;
    }
    QString styleSheet = QLatin1String(file.readAll());
    styleSheet = styleSheet.replace("$themeDir", themeDir);
    kapp->setStyleSheet(styleSheet);
    qDebug() << "Loopy: Theme: " << themeName;
}

void MainWindow::reloadTheme()
{
    loadStyleSheet(Settings::theme());
}

/////////////////////////////////////////////////////
///Trayicon
////////////////////////////////////////////////////
void MainWindow::createTrayIcon()
{
    //FIXME ->Save windowstate
    m_trayIcon = new KSystemTrayIcon(this);
    m_trayIcon->setIcon(KIcon("applications-multimedia"));
    m_trayIcon->contextMenu()->addAction(action("pause"));
    m_trayIcon->contextMenu()->addAction(action("seekforward"));
    m_trayIcon->contextMenu()->addAction(action("seekbackward"));
    m_trayIcon->contextMenu()->addAction(action("skipforward"));
    m_trayIcon->contextMenu()->addAction(action("skipbackward"));
    m_trayIcon->contextMenu()->addSeparator();
    m_trayIcon->contextMenu()->addAction(action("file_open"));
    m_trayIcon->contextMenu()->addSeparator();
    m_trayIcon->contextMenu()->addAction(action("toggleViewMode"));
    m_trayIcon->contextMenu()->addAction(action("playListAction"));
    m_trayIcon->contextMenu()->addSeparator();
    m_trayIcon->contextMenu()->addAction(action("options_configure"));

    connect(m_trayIcon, SIGNAL(quitSelected()), this, SLOT(close()));
    m_trayIcon->setVisible(Settings::trayIcon());
}

/////////////////////////////////////////////////////
///Player controls
////////////////////////////////////////////////////
void MainWindow::playPause()
{
    if (mediaObject->state() == Phonon::PlayingState) {
        mediaObject->pause();
    }
    else {
        if (mediaObject->currentTime() == mediaObject->totalTime()) {
            mediaObject->seek(0);
        }
        mediaObject->play();
    }
}

void MainWindow::seekForward()
{
    const qint64 new_pos = mediaObject->currentTime() + Settings::buttonSeek() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::jumpForward()
{
    const qint64 new_pos = mediaObject->currentTime() + Settings::buttonJump() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::mouseWheelSeekForward()
{
    const qint64 new_pos = mediaObject->currentTime() + Settings::mouseWheelSeek() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::seekBackward()
{
    const qint64 new_pos = mediaObject->currentTime() - Settings::buttonSeek() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::jumpBackward()
{
    const qint64 new_pos = mediaObject->currentTime() - Settings::buttonJump() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::mouseWheelSeekBackward()
{
    const qint64 new_pos = mediaObject->currentTime() - Settings::mouseWheelSeek() * 1000 ;
    if (new_pos > 0) {
        mediaObject->seek(new_pos);
    }
}

void MainWindow::skipForward()
{
    int index = hiddenPlayList.indexOf(mediaObject->currentSource()) + 1;
    if (hiddenPlayList.size() > index) {
        mediaObject->setCurrentSource(hiddenPlayList.at(index));
        mediaObject->play();
    }
}

void MainWindow::skipBackward()
{
    int index = hiddenPlayList.indexOf(mediaObject->currentSource()) - 1;
    if (index >= 0) {
        mediaObject->setCurrentSource(hiddenPlayList.at(index));
        mediaObject->play();
    }
}

/////////////////////////////////////////////////////
///Audio settings
////////////////////////////////////////////////////
void MainWindow::toggleMuted()
{
    audioOutput->setMuted(!audioOutput->isMuted());
}

void MainWindow::mutedChanged(bool muted)
{
    if (muted) {
        action("mute")->setIcon(KIcon("audio-volume-muted"));
        action("mute")->setText(i18n("Unmute"));
        muteButton->setIcon(KIcon("audio-volume-muted"));
    } else {
        action("mute")->setIcon(KIcon("audio-volume-medium"));
        action("mute")->setText(i18n("Mute"));
        muteButton->setIcon(KIcon("audio-volume-medium"));
    }
}

void MainWindow::increaseVolume()
{
    if (audioOutput->volume() < 1)
        audioOutput->setVolume(audioOutput->volume() + qreal(0.05));
}

void MainWindow::decreaseVolume()
{
    if (audioOutput->volume() > 0)
        audioOutput->setVolume(audioOutput->volume() - qreal(0.05));
}

void MainWindow::changeVolume(int volume)
{
    audioOutput->setVolume(volume * qreal(0.01));
}

void MainWindow::volumeChanged(qreal volume)
{
    int percentage = volume * 100 + qreal(0.5);
    volumeSlider->setValue(percentage);
}

/////////////////////////////////////////////////////
///Video settings
////////////////////////////////////////////////////
void MainWindow::aspectChanged(QAction *act)
{
    if (act->objectName() == "aspectAction16_9")
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio16_9);
    else if (act->objectName() == "aspectActionScale")
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
    else if (act->objectName() == "aspectAction4_3")
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatio4_3);
    else
        m_videoWidget->setAspectRatio(Phonon::VideoWidget::AspectRatioAuto);
}

void MainWindow::scaleChanged(QAction *act)
{
    if (act->objectName() == "scaleActionCrop")
        m_videoWidget->setScaleMode(Phonon::VideoWidget::ScaleAndCrop);
    else
        m_videoWidget->setScaleMode(Phonon::VideoWidget::FitInView);
}

void MainWindow::channelChanged(QAction *act)
{
    int idx = channelGroup->actions().indexOf(act);
    qDebug() << "Loopy: change audio channel to " << m_audioChannels[idx].name();
    mediaController->setCurrentAudioChannel(m_audioChannels[idx]);
}

void MainWindow::subtitleChanged(QAction *act)
{
    if (!isPlayable(mediaObject->currentSource().type())) {
        return;
    }
    setSubtitle(act);
}

void MainWindow::resizeToVideo()
{
    if (!isFullScreen() && !isMaximized() && Settings::autoResizeToVideo()) {
        resize(size() - centralWidget()->size() + m_videoWidget->sizeHint());
    }
}

/////////////////////////////////////////////////////
///Media state
////////////////////////////////////////////////////
void MainWindow::mediaStateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    // m_videoWidget->setVisible(mediaObject->hasVideo());
    // m_infoLabel->setVisible(!mediaObject->hasVideo());

    switch (newState) {
    case Phonon::ErrorState:
        qDebug() << "Loopy: MediaState:" << mediaObject->errorString();
        KMessageBox::error(this, mediaObject->errorString(), "Phonon");
        break;
    case Phonon::PlayingState:
        qDebug() << "Loopy: MediaState: Playing";
        resizeToVideo();
        action("pause")->setIcon(KIcon("media-playback-pause"));
        action("pause")->setText(i18n("Pause"));
        playPauseButton->setIcon(KIcon("media-playback-pause"));//fullscreenbar
        action("pause")->setEnabled(true);
        action("seekforward")->setEnabled(true);
        action("seekbackward")->setEnabled(true);
        action("jumpforward")->setEnabled(true);
        action("jumpbackward")->setEnabled(true);
        action("skipforward")->setEnabled(true);
        action("skipbackward")->setEnabled(true);
        break;
    case Phonon::PausedState:
        qDebug() << "Loopy: MediaState: Paused";
        action("pause")->setEnabled(true);
        action("pause")->setIcon(KIcon("media-playback-start"));
        action("pause")->setText(i18n("Play"));
        playPauseButton->setIcon(KIcon("media-playback-start"));
        break;
    case Phonon::StoppedState:
        qDebug() << "Loopy: MediaState: Stopped";
        action("pause")->setEnabled(true);
        action("pause")->setIcon(KIcon("media-playback-start"));
        action("pause")->setText(i18n("Play"));
        playPauseButton->setIcon(KIcon("media-playback-start"));
        /*if (!action("repeatAction")->isChecked()) {
            m_videoWidget->setVisible(false);
            m_infoLabel->setVisible(true);
        }*/
        break;
    case Phonon::BufferingState:
        qDebug() << "Loopy: MediaState: Buffering";
        break;
    default:
        ;
    }
}

void MainWindow::hasVideoChanged(bool hasVideo)
{
    qDebug() << "Loopy: hasVideo =" << (hasVideo?"true":"false");
    m_videoWidget->setVisible(hasVideo);
    m_infoLabel->setVisible(!hasVideo);
}

void MainWindow::updateCaption()
{
    QString mediatitle;
    QStringList strings = mediaObject->metaData(Phonon::TitleMetaData);

    if (!strings.isEmpty() && !strings.at(0).isEmpty()) {
        mediatitle = strings.at(0);
    }
    else {
        mediatitle = KUrl(mediaObject->currentSource().url()).fileName();
    }
    setCaption(mediatitle);
    m_titleLabelFullScreenBar->setText(mediatitle);
    m_trayIcon->setToolTip(mediatitle);
}

void MainWindow::sourceChanged(const Phonon::MediaSource &source)//File changed
{
    if (!source.url().isEmpty()) {
        qDebug() << "Loopy: Source changed" << source.url();
        m_playlistDock->visiblePlayList->setCurrentRow(hiddenPlayList.indexOf(source));
        m_playlistDock->visiblePlayList->currentItem()->setIcon(KIcon("task-complete"));
        actionOpenRecent->addUrl(source.url());
        updateCaption();
    }
}

void MainWindow::aboutToFinish()//Enqueue file
{
    qDebug() << "Loopy: About to Finish";
    int index = hiddenPlayList.indexOf(mediaObject->currentSource()) + 1;
    if (hiddenPlayList.size() > index) {
        mediaObject->enqueue(hiddenPlayList.at(index));
    }
    else if (hiddenPlayList.size() >= 2 && action("repeatAction")->isChecked()) {
        mediaObject->enqueue(hiddenPlayList.at(0));
        mediaObject->play();
    }
}

void MainWindow::finished()//Repeat file/playlist
{
    qDebug() << "Loopy: Finished";
    if (hiddenPlayList.size() <= 1 && action("repeatAction")->isChecked()) {
        mediaObject->seek(0);
        mediaObject->play();
    }
}

void MainWindow::updateAudioChannelsMenu()
{
    qDebug() << "Loopy: update audio channels";
    QMenu *channelMenu = static_cast<QMenu*>(guiFactory()->container("channelmenu", this));
    if (!channelMenu) {
        m_audioChannels.clear();
        return;
    }
    
    if (!isPlayable(mediaObject->currentSource().type())) {
        m_audioChannels.clear();
        return;
    }

    Phonon::AudioChannelDescription currentChannel =
        mediaController->currentAudioChannel();
    m_audioChannels = mediaController->availableAudioChannels();
    
    QList<QAction*> actions = channelGroup->actions();
    qDeleteAll(actions.begin(), actions.end());
    if (m_audioChannels.size() == 0)
        return;

    QAction *focus_act = NULL;
    foreach( Phonon::AudioChannelDescription channel, m_audioChannels ) {
        QAction *act = channelGroup->addAction(channel.name());
        qDebug() << channel;
        act->setCheckable(true);
        if ( channel == currentChannel ) {
            focus_act = act;
        }
        channelMenu->addAction(act);
    }

    if (!focus_act) { // ususally, the first channel is mute
        focus_act = channelGroup->actions().last();
    }
    focus_act->setChecked(true);
    channelChanged(focus_act);
}

void MainWindow::updateSubtitlesMenu()
{
    qDebug() << "Loopy: available subtitles changed";
    QMenu *subtitlesMenu = static_cast<QMenu*>(guiFactory()->container("subtitles", this));

    if (!isPlayable(mediaObject->currentSource().type())) {
        m_subtitles.clear();
        return;
    }
    
    m_subtitles = mediaController->availableSubtitles();
    Phonon::SubtitleDescription currentSubtitle = mediaController->currentSubtitle();
    
    // subtitlesMenu->setEnabled( m_subtitles.size() > 0 );
    QList<QAction*> actions = subtitlesGroup->actions();
    qDeleteAll(actions.begin(), actions.end());

    QAction *act = subtitlesGroup->addAction("externalSubtitle");
    act->setText(i18n("Load External Subtitle..."));
    act->setEnabled( isPlayable(mediaObject->currentSource().type()) );
    subtitlesMenu->addAction(act);
    
    if ( m_subtitles.size() == 0 )
        return;
    
    act = subtitlesGroup->addAction("subtitleAuto");
    act->setText(i18n("Auto Select Subtitle"));
    act->setCheckable(true);
    QAction *focus_act = act;
    subtitlesMenu->addAction(act);

    subtitlesMenu->addSeparator();
    
    foreach( Phonon::SubtitleDescription sd, m_subtitles ) {
        act = subtitlesGroup->addAction(sd.name());
        act->setCheckable(true);
        if ( sd == currentSubtitle ) {
            focus_act = act;
        }
        subtitlesMenu->addAction(act);
    }

    focus_act->setChecked(true);
    setSubtitle(focus_act);
}

void MainWindow::setSubtitle(QAction *act)
{
    int idx = subtitlesGroup->actions().indexOf(act);
    if ( idx == 0 ) { // load external
        loadSubtitle();
        return;
        
    } else if ( idx > 0 ) {
        if ( idx == 1 )
            idx = m_subtitles.size()-1;
        else
            idx -= 2;
    }

    if ( mediaController->currentSubtitle() == m_subtitles[idx] )
        return;
    
    qDebug() << "Loopy: change subtitle to " << m_subtitles[idx].name();    
    mediaController->setCurrentSubtitle(m_subtitles[idx]);
}

void MainWindow::loadSubtitle()
{
    qDebug() << "Loopy: load external subtitle ";

    // QStringList mimeTypes;
    // mimeTypes << "application/x-subrip" << "text/plain";
	//See http://en.wikipedia.org/wiki/Subtitle_(captioning)#Subtitle_formats
    QStringList suffixes = QStringList() << "aqt" << "sub" << "rt" << "smi"
                                         << "sami" << "ssf" << "srt" << "ssa"
                                         << "ass" << "usf" << "utf" << "txt";

    QString filter;
    foreach(const QString& suf, suffixes) {
        filter += " *." + suf;
    }
    filter = tr("%1|Subtitle\n*.*|All Files").arg(filter);

    KUrl url = KFileDialog::getOpenFileName(KUrl("kfiledialog:///loopy")
                                            , filter
                                            , this
                                            , i18n("Select subtitle file"));

    if (url.isValid()) {
        QHash<QByteArray, QVariant> props;
        props.insert("type", "file");
        props.insert("name", url.toLocalFile());
        //FIXME: currently there is no way to find the next available index, so
        //guess an unused one
        int max = m_subtitles.size()-1;
        foreach( const Phonon::SubtitleDescription& sd, m_subtitles ) {
            max = qMax( max, sd.index() );
        }
        Phonon::SubtitleDescription sub(max+1, props);
        mediaController->setCurrentSubtitle(sub);
    }
}

/////////////////////////////////////////////////////
///DVD
////////////////////////////////////////////////////
void MainWindow::titleCountChanged(int count)
{
    titleCount = count;
    updateTitleMenu();
}

void MainWindow::updateTitleMenu()
{
    QMenu *titleMenu = static_cast<QMenu*>(guiFactory()->container("titlemenu", this));

    if (titleCount > 1) {
        QList<QAction *> actions = titleGroup->actions();

        if (actions.count() < titleCount) {
            for (int i = actions.count(); i < titleCount; ++i) {
                QAction *action = titleGroup->addAction(QString::number(i + 1));
                action->setCheckable(true);
                titleMenu->addAction(action);
            }
        }
        else if (actions.count() > titleCount) {
            for (int i = actions.count(); i > titleCount; --i) {
                delete actions.at(i - 1);
            }
        }

        int current = mediaController->currentTitle() - 1;

        if ((current >= 0) && (current < titleCount)) {
            titleGroup->actions().at(current)->setChecked(true);
        }
        titleMenu->setEnabled(true);
    }
    else {
        titleMenu->setEnabled(false);
    }
}

void MainWindow::changeTitle(QAction *action)
{
    mediaController->setCurrentTitle(titleGroup->actions().indexOf(action) + 1);
}

void MainWindow::chapterCountChanged(int count)
{
    chapterCount = count;
    updateChapterMenu();
}

void MainWindow::updateChapterMenu()
{
    QMenu *chapterMenu = static_cast<QMenu*>(guiFactory()->container("chaptermenu", this));

    if (chapterCount > 1) {
        QList<QAction *> actions = chapterGroup->actions();

        if (actions.count() < chapterCount) {
            for (int i = actions.count(); i < chapterCount; ++i) {
                QAction *action = chapterGroup->addAction(QString::number(i + 1));
                action->setCheckable(true);
                chapterMenu->addAction(action);
            }
        }
        else if (actions.count() > chapterCount) {
            for (int i = actions.count(); i > chapterCount; --i) {
                delete actions.at(i - 1);
            }
        }

        int current = mediaController->currentChapter() - 1;

        if ((current >= 0) && (current < chapterCount)) {
            chapterGroup->actions().at(current)->setChecked(true);
        }
        chapterMenu->setEnabled(true);
    }
    else {
        chapterMenu->setEnabled(false);
    }
}

void MainWindow::changeChapter(QAction *action)
{
    mediaController->setCurrentChapter(chapterGroup->actions().indexOf(action) + 1);
}

void MainWindow::angleCountChanged(int count)
{
    angleCount = count;
    updateAngleMenu();
}

void MainWindow::updateAngleMenu()
{
    QMenu *angleMenu = static_cast<QMenu*>(guiFactory()->container("anglemenu", this));

    if (angleCount > 1) {
        QList<QAction *> actions = angleGroup->actions();

        if (actions.count() < angleCount) {
            for (int i = actions.count(); i < angleCount; ++i) {
                QAction *action = angleGroup->addAction(QString::number(i + 1));
                action->setCheckable(true);
                angleMenu->addAction(action);
            }
        }
        else if (actions.count() > angleCount) {
            for (int i = actions.count(); i > angleCount; --i) {
                delete actions.at(i - 1);
            }
        }

        int current = mediaController->currentAngle() - 1;

        if ((current >= 0) && (current < angleCount)) {
            angleGroup->actions().at(current)->setChecked(true);
        }
        angleMenu->setEnabled(true);
    }
    else {
        angleMenu->setEnabled(false);
    }
}

void MainWindow::changeAngle(QAction *action)
{
    mediaController->setCurrentAngle(angleGroup->actions().indexOf(action) + 1);
}
