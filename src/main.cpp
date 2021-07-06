#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QListWidget>
#include <QMessageLogger>
#include <QFile>
#include <QSharedMemory>
#include <QSystemSemaphore>

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

        QSystemSemaphore semaphore("13c3fb0c-9e8f-4c97-8632-07cede1e2054", 1);
        semaphore.acquire();

    #ifndef Q_OS_WIN32
        QSharedMemory nix_fix_shared_memory("<e504a879-2157-4611-ad0a-ad185c526422");
        if (nix_fix_shared_memory.attach())
            nix_fix_shared_memory.detach();
    #endif

        QSharedMemory sharedMemory("e504a879-2157-4611-ad0a-ad185c526422");
        bool is_running;
        if (sharedMemory.attach())
            is_running = true;
        else
        {
            sharedMemory.create(1);
            is_running = false;
        }
        semaphore.release();

        if (is_running)
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("The application is already running.\n"
                           "Allowed to run only one instance of the application.");
            msgBox.exec();
            return 1;
        }

        qInfo() << "============================================";
        qInfo() << "Application started";
        MainWindow w;
        w.setWindowState(Qt::WindowMaximized);
        w.show();

        a.exec();
    }

    qInfo() << "Application closed";

    return 0;
}
