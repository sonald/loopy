#include <KAboutData>
#include <KCmdLineArgs>
#include <KUniqueApplication>

#include "mainwindow.h"

class LoopyApplication : public KUniqueApplication
{
public:
    LoopyApplication() : firstInstance(true) {
        loopy = new MainWindow();
        loopy->show();
        loopy->parseArgs();
    }

    ~LoopyApplication() {
        // unlike qt, kde sets Qt::WA_DeleteOnClose and needs it to work properly ...
    }

private:
    int newInstance();

    MainWindow *loopy;
    bool firstInstance;
};

int LoopyApplication::newInstance()
{
    if (firstInstance) {
        // using KFileWidget, newInstance() might be called _during_ loopy construction
        firstInstance = false;
    }
    else {
        loopy->parseArgs();
    }

    return KUniqueApplication::newInstance();
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("loopy", "loopy",
                         ki18n("Loopy"), "0.5.3",
                         ki18n("Slim, themeable video player, based on the KDE phonon libraries."),
                         KAboutData::License_GPL_V2,
                         ki18n("Copyright (c) 2010 MonsterMagnet"));

    aboutData.setProgramIconName("applications-multimedia");
    aboutData.addAuthor(ki18n("MonsterMagnet"), ki18n("Original Developer/Maintainer"), "monstermagnet@rocketmail.com");
    aboutData.addCredit(ki18n("The Kaffeine Developers"),ki18n("Single instance, DVD support and a lot more"));
    aboutData.addCredit(ki18n("The Gwenview Developers"),ki18n("Fullscreenbar and qss stylesheet"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(MainWindow::cmdLineOptions());

    LoopyApplication app;
    return app.exec();
}
