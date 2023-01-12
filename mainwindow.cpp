#include "mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QChar>
#include <QString>
#include <QRegExp>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    int minWidth = 500;
    int minHeight = 500;

    /* Setup QAbstractItemModel & QTreeView */

    QStringList args = QCoreApplication::arguments();

    if(args.size() != 3){

        QString errStr =    R"(Program takes exactly 2 Arguments:
AmsNetId [STRING], root folder [STRING]
E.g.:
5.69.55.236.1.1 C:/TwinCAT/
5.80.201.232.1.1 /usr/local/etc/TwinCAT/)";

        QMessageBox messageBox;
        messageBox.setText(errStr);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
        exit(EXIT_FAILURE);
    }

    QString AmsNetId = QCoreApplication:: arguments().at(1);
    QString rootDir  = QCoreApplication::arguments().at(2);


    QRegExp rxNetId("^(?:[0-9]{1,3}\\.){5}[0-9]{1,3}$");

    if(!rxNetId.exactMatch(AmsNetId)){
        qWarning() << "Can't process AmsNetId. A AmsNet consist of exactly 6 bytes, e.g.:";
        qWarning() << "5.69.55.236.1.1";
        exit(EXIT_FAILURE);
    }

    m_adsNodeModel = new AdsFileSystemModel(rootDir, AmsNetId);

    /* Setup TreeView */
    m_treeView.setModel(m_adsNodeModel);
    m_treeView.setDragEnabled(true);
    m_treeView.viewport()->setAcceptDrops(true);
    m_treeView.setDropIndicatorShown(true);
    m_treeView.setColumnWidth(0, minWidth / 1.3);

    /* Setup Download Button */
    m_downloadBtn.setText("Download");
    connect(&m_downloadBtn, &QPushButton::clicked, &m_treeView, &AdsFileTree::downloadSelected);

    /* Setup Main Layout */

    m_mainLayout.setSpacing(0);
    m_mainLayout.setContentsMargins(0, 0, 0, 0);

    m_mainLayout.addWidget(&m_treeView);
    m_mainLayout.addWidget(&m_downloadBtn);
    m_centralWidget.setLayout(&m_mainLayout);

    this->setCentralWidget(&m_centralWidget);
    this->setWindowTitle(QStringLiteral("ADS File Browser"));
    this->setMinimumSize(minWidth, minHeight);
}

MainWindow::~MainWindow()
{
}
