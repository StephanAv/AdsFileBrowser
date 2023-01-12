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
    connect(m_adsNodeModel, &AdsFileSystemModel::download, this, &MainWindow::processDownload);

    /* Setup TreeView */
    m_treeView.setModel(m_adsNodeModel);
    m_treeView.setDragEnabled(true);
    m_treeView.viewport()->setAcceptDrops(true);
    m_treeView.setDropIndicatorShown(true);
    m_treeView.setColumnWidth(0, minWidth / 1.3);
    connect(&m_treeView, &AdsFileTree::download, m_adsNodeModel, &AdsFileSystemModel::downloadFile);

    /* Setup Download Button */
    m_downloadBtn.setText("Download");
    //connect(&m_downloadBtn, &QPushButton::clicked, this, &MainWindow::showProgress);
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

    //m_progressDialog.setParent(this);
    m_progressDialog.cancel();
    m_progressDialog.setMinimum(0);
    m_progressDialog.setMaximum(100);
    m_progressDialog.setMinimumDuration(500);
    m_progressDialog.setAutoClose(true);
    m_progressDialog.setLabelText(QStringLiteral("Downloading files..."));
    m_progressDialog.setCancelButtonText(QStringLiteral("Abort"));
    m_progressDialog.setWindowModality(Qt::WindowModal);
    //m_progressBar.setParent(this);
    //m_progressBar.show();
    //m_progressBar.exec();
}

void MainWindow::processDownload(QString remoteFile)
{
    qDebug() << "processDownload() called";


//    QProgressDialog *progress = new QProgressDialog("Downloading files...", "Abort", 0, 100, this);
//    progress->setWindowModality(Qt::WindowModal);

//    for (int i = 0; i < numFiles; i++) {
//        progress.setValue(i);

//        if (progress.wasCanceled())
//            break;
//        //... copy one file
//    }
//    progress.setValue(numFiles);

    //void (*barValue)(int p) = [&](int p) { progress.setValue(p); };

    int fileNamePos = remoteFile.lastIndexOf('/');
    QStringRef fName(&remoteFile, fileNamePos, remoteFile.length() - fileNamePos);

    QString localFilePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    localFilePath.append(fName);

    std::ofstream localFile(localFilePath.toStdString().c_str(), std::ios::binary);
    m_progressDialog.reset();
    //m_progressDialog.open(this, "downloadCanceled");
    m_progressDialog.show();
    //m_progressDialog.setValue(12);
    //void (*barValue)(int p) = m_progressDialog.setValue;
    //connect( &m_progressDialog, &QProgressDialog::)
    qint32 err = m_adsNodeModel->m_fso->readDeviceFile(remoteFile.toStdString().c_str(), localFile, this->m_progressDialog.setValue);
    m_adsNodeModel->handleError(err);
}


void MainWindow::makeProgress(int progress)
{
    qDebug() << "Progress: " << progress;

    //emit progressMade(progress);
    //m_progressDialog.setValue(progress);
}

void MainWindow::downloadCanceled()
{
    qDebug() << "downloadCanceled";
}
void MainWindow::showProgress()
{
    qDebug() << "MainWindow::showProgress() clicked";
    //m_progressBar.exec();
    //m_progressBar.show();
}


MainWindow::~MainWindow()
{

}
