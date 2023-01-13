#include "mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QChar>
#include <QString>
#include <QRegExp>
#include <QMessageBox>
#include <QStandardPaths>

#include <functional>

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

    std::function<size_t(QString, QString)> fUpload = [=](QString a, QString b){
        return this->processUpload(a, b);
    };

    m_adsNodeModel = new AdsFileSystemModel(rootDir, AmsNetId, fUpload);
    connect(m_adsNodeModel, &AdsFileSystemModel::download, this, &MainWindow::processDownload);
    //connect(m_adsNodeModel, &AdsFileSystemModel::upload, this, &MainWindow::processUpload);

    /* Setup TreeView */
    m_treeView.setModel(m_adsNodeModel);
    m_treeView.setDragEnabled(true);
    m_treeView.viewport()->setAcceptDrops(true);
    m_treeView.setDropIndicatorShown(true);
    m_treeView.setColumnWidth(0, minWidth / 1.3);
    connect(&m_treeView, &AdsFileTree::download, m_adsNodeModel, &AdsFileSystemModel::downloadFile);

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

size_t MainWindow::processUpload(QString localFile, QString targetFile)
{
    qDebug() << "processUpload!";
    std::ifstream source(localFile.toStdString(), std::ios::binary); // TODO

    int fileNamePos = localFile.lastIndexOf('/');
    QStringRef fName(&localFile, fileNamePos, localFile.length() - fileNamePos);

    QProgressDialog *progress = new QProgressDialog(QString("Uploading ").append(fName.right(fName.size()-1)), "Abort", 0, 100, this);
    connect(progress, &QProgressDialog::canceled, this, &MainWindow::transferCanceled);
    std::function<void(int)> fBar = [&](int x){  progress->setValue(x) ;};
    progress->open();

    m_cancelTransfer = false;
    qint32 err = m_adsNodeModel->m_fso->writeDeviceFile(targetFile.toStdString().c_str(), source, fBar, m_cancelTransfer);

    progress->reset();
    m_adsNodeModel->handleError(err);

    return 0;
}

void MainWindow::processDownload(QString remoteFile)
{
    int fileNamePos = remoteFile.lastIndexOf('/');
    QStringRef fName(&remoteFile, fileNamePos, remoteFile.length() - fileNamePos);

    QString localFilePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    localFilePath.append(fName);

    std::ofstream localFile(localFilePath.toStdString().c_str(), std::ios::binary);

    QProgressDialog *progress = new QProgressDialog(QString("Downloading ").append(fName.right(fName.size()-1)), "Abort", 0, 100, this);
    connect(progress, &QProgressDialog::canceled, this, &MainWindow::transferCanceled);
    std::function<void(int)> fBar = [&](int x){  progress->setValue(x) ;};
    progress->open();

    m_cancelTransfer = false;
    qint32 err = m_adsNodeModel->m_fso->readDeviceFile(remoteFile.toStdString().c_str(), localFile, fBar, m_cancelTransfer);

    progress->reset();
    m_adsNodeModel->handleError(err);
}

void MainWindow::transferCanceled()
{
    m_cancelTransfer = true;
}

MainWindow::~MainWindow()
{

}
