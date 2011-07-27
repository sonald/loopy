#include <KConfigDialog>
#include <KStandardDirs>

#include "mainwindow.h"
#include "ui_themeoptions.h"
#include "ui_seekingoptions.h"
#include "ui_generaloptions.h"

class SeekingOptions : public QWidget, public Ui::SeekingOptions
{
public:
    SeekingOptions(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};

class ThemeOptions : public QWidget, public Ui::ThemeOptions
{
public:
    ThemeOptions(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};

class GeneralOptions : public QWidget, public Ui::GeneralOptions
{
public:
    GeneralOptions(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};

void MainWindow::showSettings()
{
    if (KConfigDialog::showDialog("settings"))
        return;

    m_seekingConfigDialog = new SeekingOptions();
    m_themeConfigDialog = new ThemeOptions();
    m_generalConfigDialog = new GeneralOptions();

    //Create a list of available themes
    QStringList list;
    const QStringList themeBaseDirs = KGlobal::mainComponent().dirs()->findDirs("appdata", "themes");
    Q_FOREACH(const QString& themeBaseDir, themeBaseDirs) {
        QDir dir(themeBaseDir);
        list += dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    m_themeConfigDialog->kcfg_Theme->addItem(i18n("no theme"));
    m_themeConfigDialog->kcfg_Theme->addItems(list);

    int row = list.indexOf(Settings::theme());
    m_themeConfigDialog->kcfg_Theme->setCurrentRow(row + 1);

    KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
    dialog->addPage(m_generalConfigDialog, i18n("General"), "configure");
    dialog->addPage(m_themeConfigDialog, i18n("Theme"), "fill-color");
    dialog->addPage(m_seekingConfigDialog, i18n("Seeking"), "media-seek-forward");
    //dialog->setInitialSize(QSize(440, 390));
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(saveSettings()));

    //If theme selection has changed enable KConfigDialog "Apply"
    QObject::connect(m_themeConfigDialog->kcfg_Theme, SIGNAL(currentTextChanged(const QString&)),
                     dialog, SLOT(updateButtons()));

    dialog->show();
}

void MainWindow::saveSettings()
{
    //Tray icon
    m_trayIcon->setVisible(m_generalConfigDialog->kcfg_TrayIcon->isChecked());

    //Autoresize to videosize
    if (!hiddenPlayList.isEmpty()) {
        resizeToVideo();
    }

    //Theme
    QString themeName = m_themeConfigDialog->kcfg_Theme->currentItem()->text();
    if (themeName.isEmpty() || themeName == i18n("no theme")) {
        loadStyleSheet("none");
        return;
    }
    Settings::setTheme(themeName);
    Settings::self()->writeConfig();
    loadStyleSheet("none");//clear current style
    loadStyleSheet(themeName);
}

