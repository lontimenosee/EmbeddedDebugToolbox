#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QWidget;
class QHBoxLayout;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class SerialWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleNavigationItemClicked(QListWidgetItem *item);

private:
    QWidget *m_centralWidget = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    QListWidget *m_navigationListWidget = nullptr;
    QStackedWidget *m_pageStackWidget = nullptr;
    SerialWidget *m_serialAssistantPage = nullptr;
};

#endif // MAINWINDOW_H
