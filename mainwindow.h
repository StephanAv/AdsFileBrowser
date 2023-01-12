#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>
#include <QComboBox>

#include "adsfilesystemmodel.h"
#include "adsfiletree.h"
#include "progressbar.h"

#include <QProgressDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    // Central Layout
    QWidget     m_centralWidget;
    QVBoxLayout m_mainLayout;

    // Control Bar
    QWidget     m_ctrlBarWidget;
    QHBoxLayout m_ctrlBarLayout;
    QPushButton m_reconnectBtn;
    QComboBox   m_targetList;

    // QTreeView
    AdsFileTree   m_treeView;

    // ADS
    AdsFileSystemModel* m_adsNodeModel;

    // Download button
    QPushButton m_downloadBtn;

    // Progress Bar
    //ProgressBar m_progressBar;
    QProgressDialog m_progressDialog;

    static void makeProgress(int progress);

signals:
    void progressMade(int progress);

public slots:
    void showProgress();
    void downloadCanceled();
    void processDownload(QString remoteFile);
};
#endif // MAINWINDOW_H
