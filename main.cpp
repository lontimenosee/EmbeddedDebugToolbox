#include "mainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTextStream>
#include <QStringLiteral>

namespace {

void appendStartupLog(const QString &message)
{
    QFile logFile(QDir::current().filePath("startup.log"));
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&logFile);
    stream << QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss] ")
           << message << '\n';
}

}

int main(int argc, char *argv[])
{
    appendStartupLog("main() entered");
    QApplication app(argc, argv);
    appendStartupLog("QApplication created");
    app.setApplicationName("Embedded Debug Toolbox");
    app.setApplicationDisplayName(QStringLiteral("\u5d4c\u5165\u5f0f\u8c03\u8bd5\u52a9\u624b"));
    app.setWindowIcon(QIcon(":/icon32.ico"));
    appendStartupLog("Application metadata configured");

    MainWindow window;
    appendStartupLog("MainWindow constructed");
    window.show();
    appendStartupLog(QString("MainWindow visible=%1").arg(window.isVisible()));

    const int result = app.exec();
    appendStartupLog(QString("app.exec() returned %1").arg(result));
    return result;
}
