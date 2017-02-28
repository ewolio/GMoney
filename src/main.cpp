#define VERSION 0.8

#include <QtGui>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QLocale::setDefault(QLocale::French);

    QPixmap pixmap(":/imgs/splash.png");
    QPainter painter(&pixmap);
    painter.setFont(QFont("Cantarell",12,0.7));
    painter.setPen(QColor("#817b7b"));
    painter.drawText(480,280,90,20, Qt::AlignCenter, "Version " + QString().setNum(VERSION));

    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents();

    MainWindow::Instance()->show();
    splash.finish(MainWindow::Instance());


    return a.exec();
}
