#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QStringLiteral>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Embedded Debug Toolbox");
    app.setApplicationDisplayName(QStringLiteral("\u5d4c\u5165\u5f0f\u8c03\u8bd5\u52a9\u624b"));
    app.setWindowIcon(QIcon(":/icon32.ico"));

    MainWindow window;
    window.show();

    return app.exec();
}
