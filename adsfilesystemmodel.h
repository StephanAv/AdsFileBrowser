#ifndef ADSFILESYSTEMMODEL_H
#define ADSFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <optional>

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

    AdsFileSystemModel(const QString& root, const QString& amsNetId);

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

    void downloadFile(QModelIndex &idx) const;

    static AmsNetId createNetId(const QString& netId);

    std::shared_ptr<AdsFileInfoNode>                        m_rootNode;
    std::shared_ptr<BasicADS>                               m_adsClient;
    std::shared_ptr<DeviceManager::FileSystemObject>        m_fso;

};

#endif // ADSFILESYSTEMMODEL_H
