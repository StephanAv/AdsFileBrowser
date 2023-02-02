#ifndef ADSFILESYSTEMMODEL_H
#define ADSFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QLocale>
#include <optional>
#include <functional>

#include "file_system_object.h"
#include "adsfileinfonode.h"

#if defined(USE_TWINCAT_ROUTER)
    #include "TC1000_AdsClient.h"
#else
    #include "GenericAdsClient.h"
#endif



class AdsFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    AdsFileSystemModel(const QString& root, const QString& amsNetId, std::function<size_t(QString, QString)> processUpload);
    ~AdsFileSystemModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void handleError(qint32 err) const;
    void setRoot(const QString& root);

    static AmsNetId createNetId(const QString& netId);

    std::shared_ptr<AdsFileInfoNode>                        m_rootNode;
    std::shared_ptr<BasicADS>                               m_adsClient;
    std::shared_ptr<DeviceManager::FileSystemObject>        m_fso;

 private:

    std::shared_ptr<AdsFileInfoNode> getPtr(const AdsFileInfoNode* rawPtr) const;

    std::function<size_t(QString, QString)> m_processUpload;


signals:
    void download(QString path);
    //void upload(QString localFile, QString targetFile);

public slots:
    void downloadFile(QModelIndex idx);
};

#endif // ADSFILESYSTEMMODEL_H
