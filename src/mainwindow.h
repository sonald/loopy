#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>
#include <kxmlguifactory.h>
#include <KMessageBox>
#include <KIcon>
#include <KMenuBar>
#include <KToolBar>
#include <KStatusBar>
#include <KFileDialog>
#include <KLocale>
#include <KCmdLineOptions>
#include <KInputDialog>
#include <KRecentFilesAction>
#include <KSystemTrayIcon>

#include <Phonon/AudioOutput>
#include <Phonon/SeekSlider>
#include <Phonon/MediaObject>
#include <Phonon/VolumeSlider>
#include <Phonon/BackendCapabilities>
#include <Phonon/MediaController>

#include <Solid/Block>
#include <Solid/Device>

#include <QList>
#include <QAction>
#include <QActionGroup>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QStringList>
#include <QToolButton>

#include "videowidget.h"
#include "playlistdock.h"
#include "titlelabel.h"
#include "timelabel.h"
#include "infolabel.h"
#include "fullscreenbar.h"
#include "fullscreenplaylist.h"

#include "tooltipeater.h"

#include "settings.h" //generated by kconfig_compiler from loopy.kcfg

class SeekingOptions;
class ThemeOptions;
class GeneralOptions;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    QList<Phonon::MediaSource> hiddenPlayList;
    Phonon::MediaObject *mediaObject;
    PlaylistDock *m_playlistDock;

public slots:
    static KCmdLineOptions cmdLineOptions();
    void openUrls(const QList<KUrl> &urls);
    void addUrls(const QList<KUrl> &urls);
    void parseArgs();
    void mouseWheelSeekForward();
    void mouseWheelSeekBackward();
    void toggleFullscreen();

private slots:
    void playPause();
    void seekForward();
    void jumpForward();
    void skipForward();
    void seekBackward();
    void jumpBackward();
    void skipBackward();
    void toggleMuted();
    void mutedChanged(bool muted);
    void changeVolume(int volume);
    void volumeChanged(qreal volume);
    void increaseVolume();
    void decreaseVolume();

    void showMenuBar();
    void toggleViewMode();

    void disableScreenSaver();
    void loadStyleSheet(QString themeName);
    void reloadTheme();

    void openFile();
    void addFile();
    void openUrl();
    void openUrl(const KUrl &url);
    void playDVD();

    void mediaStateChanged(Phonon::State newState, Phonon::State oldState);
    void sourceChanged(const Phonon::MediaSource &source);
    void hasVideoChanged(bool hasVideo);
    void aboutToFinish();
    void finished();
    void updateCaption();
    void updateSubtitlesMenu();
    void setSubtitle(QAction*);
    void updateTitleMenu();//DVD
    void updateChapterMenu();//DVD
    void updateAngleMenu();//DVD
    void chapterCountChanged(int);//DVD
    void titleCountChanged(int);//DVD
    void changeTitle(QAction*);//DVD
    void changeChapter(QAction*);//DVD
    void changeAngle(QAction*);//DVD
    void angleCountChanged(int);//DVD

    void scaleChanged(QAction *);
    void aspectChanged(QAction *);
    void subtitleChanged(QAction *);
    void resizeToVideo();

    void fullscreen(bool isFullScreen);

    //-->options.cpp
    void showSettings();
    void saveSettings();

private:
    Phonon::AudioOutput *audioOutput;
    Phonon::MediaController *mediaController;
    QList<Phonon::SubtitleDescription> m_subtitles;
    Phonon::SubtitleDescription m_currentSubtitle;
    
    KSystemTrayIcon *m_trayIcon;

    VideoWidget *m_videoWidget;

    //PlaylistDock *m_playlistDock;
    TitleLabel *m_titleLabelFullScreenBar;
    TimeLabel *m_timeLabel;
    TimeLabel *m_timeLabelFullScreenBar;
    InfoLabel *m_infoLabel;

    SeekingOptions *m_seekingConfigDialog;
    ThemeOptions *m_themeConfigDialog;
    GeneralOptions *m_generalConfigDialog;

    ToolTipEater *m_toolTipEater;

    QTimer *disableScreenSaverTimer;

    QActionGroup *subtitlesGroup;
     QActionGroup *titleGroup;//DVD
    QActionGroup *chapterGroup;//DVD
    QActionGroup *angleGroup;//DVD

    QSlider *volumeSlider;
    QToolButton *muteButton;
    QToolButton *playPauseButton;

    KRecentFilesAction *actionOpenRecent;

    void setupActions();
    void setupFullScreenToolBar();
    void createTrayIcon();
    void pushUrls(const QList<KUrl>&);
    void pushUrl(const KUrl&);
    
    bool isMainToolBar;
    bool isPlayListDock;
    int titleCount;//DVD
    int chapterCount;//DVD
    int angleCount;//DVD

};

#endif
