#include "adsfilesystemmodel.h"
#include "file_system_object.h"
#include "ads_exception.h"
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QStandardPaths>

#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <system_error>

AdsFileSystemModel::AdsFileSystemModel(const QString& root, const QString& amsNetId)
    : QAbstractItemModel()
{
    m_adsClient = std::shared_ptr<BasicADS>(new TC1000AdsClient(createNetId(amsNetId)));
    try {
        m_fso = std::shared_ptr<DeviceManager::FileSystemObject>(new DeviceManager::FileSystemObject(*m_adsClient));

    }  catch (const DeviceManager::AdsException& ex) {
        QString errStr(ex.what());
        QMessageBox messageBox;
        messageBox.setText(errStr);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
        exit(EXIT_FAILURE);
    }


    if(m_fso->operator!()){
        QMessageBox messageBox;
        QString errStr("Device Manager: File System Object not available on target");
        messageBox.setText(errStr);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
        throw std::system_error(ENOENT, std::generic_category(), errStr.toStdString());
    }

    setRoot(root);
}

void AdsFileSystemModel::setRoot(const QString& root)
{
    m_rootNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(root, FileType::Root, m_fso));

    auto firstNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(root, FileType::Folder_Initialized, m_fso, m_rootNode.get()));

    m_rootNode->m_children.push_back(firstNode);

    // Fill first node

    std::vector<std::string> folders;
    std::vector<std::string> files;

    QString search_path = root + "*";
    quint32 ret = m_fso->dir(search_path.toStdString().c_str(), folders, files);
    handleError(ret);

    if(ret != ADSERR_NOERR){
        QString errStr = QString("Specified root folder (%1) not found on target").arg(root);
        throw std::system_error(ENOENT, std::generic_category(), errStr.toStdString());
    }

    for(const auto& folder : folders){
        QString subFolderPath = root + QString(folder.c_str()) + QStringLiteral("/");
        auto subFolderNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(subFolderPath, FileType::Folder, m_fso, firstNode.get()));
        firstNode->m_children.push_back(subFolderNode);

    }

    for(const auto& file : files){
        QString filePath = root + QString(file.c_str());
        auto fileNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(filePath, FileType::File, m_fso, firstNode.get()));
        firstNode->m_children.push_back(fileNode);

    }
}

void AdsFileSystemModel::downloadFile(QModelIndex &idx) const
{
    AdsFileInfoNode* node = reinterpret_cast<AdsFileInfoNode*>(idx.internalPointer());
    if(node && node->m_type == FileType::File){

        int fileNamePos = node->m_path.lastIndexOf('/');
        QStringRef fName(&node->m_path, fileNamePos, node->m_path.length() - fileNamePos);

        QString localFilePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        localFilePath.append(fName);

        std::ofstream localFile(localFilePath.toStdString().c_str(), std::ios::binary);

        qint32 err = m_fso->readDeviceFile(node->m_path.toStdString().c_str(), localFile);
        handleError(err);
    }
}

AmsNetId AdsFileSystemModel::createNetId(const QString &netId)
{
    // Parse AmsNetId

    uint8_t b_netId[6]  = { 0, 0, 0, 0, 1, 1 };
    size_t i = 0;
    std::istringstream s_amsAddr(netId.toStdString());
    std::string token;

    while((i < sizeof(b_netId)) && std::getline(s_amsAddr, token, '.')){
        try {
            b_netId[i++] = std::stoi(token);
        } catch (std::logic_error const& ex) {
            std::string err("Error parsing AmsNetId: ");
            err += ex.what();
            // TODO: Throw!
        }
    }
    //AmsNetId remoteNetId{ b_netId[0], b_netId[1], b_netId[2], b_netId[3], b_netId[4], b_netId[5] };
    return AmsNetId{ b_netId[0], b_netId[1], b_netId[2], b_netId[3], b_netId[4], b_netId[5] };
}

int AdsFileSystemModel::rowCount(const QModelIndex &parent) const
{
    AdsFileInfoNode* node = Q_NULLPTR;
    if(parent.isValid()){
        node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
    } else {
        node = m_rootNode.get();
    }
    if (node->m_type == FileType::Folder){

        std::vector<std::string> folders;
        std::vector<std::string> files;

        QString search_path = node->m_path + "*";
        qint32 ret = m_fso->dir(search_path.toStdString().c_str(), folders, files);
        handleError(ret);

        node->m_type = FileType::Folder_Initialized;

        for(const auto& folder : folders){
            QString subFolderPath = node->m_path + QString(folder.c_str()) + QStringLiteral("/"); // TODO: Platform spezifisches Trennzeichen einfuegen
            auto subFolderNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(subFolderPath, FileType::Folder, m_fso, node));
            node->m_children.push_back(subFolderNode);

        }

        for(const auto& file : files){
            QString filePath = node->m_path + QString(file.c_str());
            auto fileNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(filePath, FileType::File, m_fso, node));
            node->m_children.push_back(fileNode);

        }
    }

    return node->m_children.count();
}

int AdsFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex AdsFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    AdsFileInfoNode* node = Q_NULLPTR;
    QModelIndex idx;

    node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
    if(!node){
        node = m_rootNode.get();
    }

    if( (node->m_type == FileType::Folder_Initialized ||
         node->m_type == FileType::Root)
         &&
         row >= 0)
    {
        if(row < node->m_children.count()){
            AdsFileInfoNode* childAtRow = node->m_children[row].get();
            idx = createIndex(row, column, childAtRow);
        }


    } else {
        qDebug() << "QModelIndex invalid for " << node->m_path;
    }
    return idx;
}

QModelIndex AdsFileSystemModel::parent(const QModelIndex &index) const
{

    AdsFileInfoNode* node = Q_NULLPTR;
    if(!index.isValid()){
        return QModelIndex();
    }

    node = reinterpret_cast<AdsFileInfoNode*>(index.internalPointer());

    if(!node){
        return QModelIndex();
    }

    AdsFileInfoNode* parent = Q_NULLPTR;

    parent = node->m_parent;

    if(!parent){
        return QModelIndex();
    }

    // Jetzt: Row von parent herausfinden

    AdsFileInfoNode* grandparent = Q_NULLPTR;

    grandparent = parent->m_parent;

    if(!grandparent){
        return QModelIndex();
    }


    int row = 0;
    for(int i = 0; i < grandparent->m_children.count(); i++){
        if(grandparent->m_children[i].get() == parent){
            row = i;
            break;
        }
    }

    return createIndex(row, 0, parent);
}

QVariant AdsFileSystemModel::data(const QModelIndex &index, int role) const
{
    AdsFileInfoNode* node = Q_NULLPTR;
    if(index.isValid()){
        node = reinterpret_cast<AdsFileInfoNode*>(index.internalPointer());
    } else {
        node = m_rootNode.get();
    }

    if(role == 0){
        return QVariant(node->m_path);
    } else {
        return QVariant();
    }
}

QVariant AdsFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    QVariant ret;
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        ret = QVariant("Path");
    }
    return ret;
}

Qt::ItemFlags AdsFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if(index.isValid()){
        return defaultFlags | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    } else {
        return defaultFlags;
    }

}

bool AdsFileSystemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    if(action == Qt::CopyAction){
        return true;
    } else {
        return false;
    }
}

bool AdsFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)

    AdsFileInfoNode* node = Q_NULLPTR;
    QModelIndex _parent = parent;

    if(_parent.isValid()){
        node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
    } else {
        return false;
    }

    if(!node){
        return false;
    }

    if(node->m_type == FileType::File){
        node = node->m_parent;
        _parent = parent.parent();
    }


    if(data->hasUrls()){
        const QList<QUrl> urls = data->urls();
        for(const auto& url : qAsConst(urls) ){

            QString localFile = url.toLocalFile();
            std::ifstream source(url.toLocalFile().toStdString(), std::ios::binary);
            QString target = node->m_path + url.fileName();

            qint32 ret = m_fso->writeDeviceFile(target.toStdString().c_str(), source);
            handleError(ret);

            auto ftest = [&](std::shared_ptr<AdsFileInfoNode> in){ return in->m_path == target; };
            bool nodeAlreadyExist = std::find_if(node->m_children.begin(), node->m_children.end(), ftest) != node->m_children.end();

            if(ret == ADSERR_NOERR && !nodeAlreadyExist){

                int target_row = node->m_children.count() - 1;
                beginInsertRows(_parent, target_row, target_row);

                auto newNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(target, FileType::File, m_fso, node));
                node->m_children.append(newNode);

                endInsertRows();
            }
        }
    } else {
        return false;
    }

    return true;
}

bool AdsFileSystemModel::removeRows(int row, int count, const QModelIndex &parent){

    Q_UNUSED(count)
    qDebug() << "Remove Rows called!!! row: " << row;

    AdsFileInfoNode* node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
    if(!node) return false;

    AdsFileInfoNode* parentNode = reinterpret_cast<AdsFileInfoNode*>(node->m_parent);

    if(!parentNode) return false;

    quint32 ret = m_fso->deleteFile(node->m_path.toStdString().c_str());
    handleError(ret);

    if(ret != ADSERR_NOERR) return false;

    beginRemoveRows(parent.parent(), row, row);
    parentNode->m_children.remove(row);
    endRemoveRows();

    return true;
}

void AdsFileSystemModel::handleError(qint32 err) const
{
    if(err == ADSERR_NOERR) return;
    qlonglong _err = 0xFFFFFFFF;
    _err = _err & err;
    QString errStr = QString("Error occured: 0x%1").arg(_err, 8, 16);

    QMessageBox messageBox;
    messageBox.setText(errStr);
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.exec();
}
