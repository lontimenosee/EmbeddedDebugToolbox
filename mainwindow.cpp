#include "mainwindow.h"

#include "serialWidget/serialwidget.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Embedded Debug Toolbox");
    setWindowIcon(QIcon(":/icon32.ico"));
    resize(1200, 760);

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_navigationListWidget = new QListWidget(this);
    m_navigationListWidget->setFixedWidth(160);
    m_navigationListWidget->addItem("Serial Assistant");

    m_pageStackWidget = new QStackedWidget(this);
    m_serialAssistantPage = new SerialWidget(this);
    m_pageStackWidget->addWidget(m_serialAssistantPage);

    m_mainLayout->addWidget(m_navigationListWidget);
    m_mainLayout->addWidget(m_pageStackWidget);

    connect(m_navigationListWidget,
            &QListWidget::itemClicked,
            this,
            &MainWindow::handleNavigationItemClicked);

    m_navigationListWidget->setCurrentRow(0);
    m_pageStackWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::handleNavigationItemClicked(QListWidgetItem *item)
{
    const int pageIndex = m_navigationListWidget->row(item);
    if (pageIndex < 0 || pageIndex >= m_pageStackWidget->count()) {
        return;
    }

    m_pageStackWidget->setCurrentIndex(pageIndex);
}
