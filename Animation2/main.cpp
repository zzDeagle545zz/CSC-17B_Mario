#include <QApplication>
#include <QDateTime>
#include <QGraphicsView>
#include <QSplashScreen>
#include <QMediaPlayer>
#include "myscene.h"
#include<QTimer>
#include<QToolBar>
#include "mainwindow.h"
#include "dialog.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QMediaPlayer * music = new QMediaPlayer();
    music->setMedia(QUrl("qrc:/music/ThemeSong.mp3"));
    //music->play();

    qsrand(QDateTime::currentMSecsSinceEpoch());
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/images/mariosplash.png"));
    splash->show();

    Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;
    splash->showMessage(QObject::tr("Setting up the main window..."),
                        topRight, Qt::white);
    MainWindow Mainwin;
    MyScene scene;
    QGraphicsView view;

    view.setRenderHint(QPainter::Antialiasing);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setScene(&scene);
    view.setFixedSize(scene.sceneRect().size().toSize());

    Mainwin.setWindowTitle("Mario");
    Mainwin.setCentralWidget(&view);
    Mainwin.setFixedSize(800,370);

    QTimer::singleShot(5000,splash,SLOT(close()));
    QTimer::singleShot(5000,&Mainwin,SLOT(show()));
    //Mainwin.show();


    return a.exec();
}

