#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QFontDatabase>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    w.show();

    return a.exec();
}
