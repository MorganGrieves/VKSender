#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QFontDatabase>
#include <QMessageLogger>

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QMessageLogger>
#include <QtCore/QMessageLogContext>
#include <QtCore/QtMessageHandler>
#include <QtWidgets/QApplication>
#include <qapplication.h>

void debugMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString timestr;
    timestr = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ");
    QByteArray txt;
    txt += "[ " + timestr + " ] ";

    switch ( type )
    {
        case QtInfoMsg:
            txt += "[INFO] ";
            txt += QString("\t %1").arg(msg);
            break;
        case QtDebugMsg:
            txt += "[DEBUG] ";
            txt += context.file;
            txt += ":";
            txt += QString("%1").arg(context.line);
            txt += QString(" - %1").arg(msg);
            break;
        case QtWarningMsg:
            txt += "[WARNING] ";
            txt += context.function;
            txt += ":";
            txt += context.line;
            txt += QString(" - \t %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt += "[CRITICAL] ";
            txt += context.function;
            txt += ":";
            txt += context.line;
            txt += QString(" - \t %1").arg(msg);
            break;
        case QtFatalMsg:
            txt += "[FATAL] ";
            txt += context.function;
            txt += ":";
            txt += context.line;
            txt += QString(" - \t %1").arg(msg);
            abort();
    }

    QFile outFile("vksender.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream textStream(&outFile);
    textStream << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(debugMessageOutput);

    {
        QApplication a(argc, argv);
        qInfo() << "============================================";
        qInfo() << "Application started";
        MainWindow w;

        w.show();

        a.exec();
    }

    qInfo() << "Application closed";

    return 0;
}
