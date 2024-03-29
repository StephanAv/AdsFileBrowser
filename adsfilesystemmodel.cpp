#include "adsfilesystemmodel.h"
#include "file_system_object.h"
#include "ads_exception.h"
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QDir>

#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <system_error>

AdsFileSystemModel::AdsFileSystemModel(const QString& root, const QString& amsNetId, std::function<size_t(QString, QString)> processUpload)
    : QAbstractItemModel()
    , m_rootPath(root)
    , m_processUpload(processUpload)
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

    setRoot();
}

AdsFileSystemModel::~AdsFileSystemModel()
{
    //qDebug() << __func__;
}

void AdsFileSystemModel::setRoot()
{
    QDir rootDir(m_rootPath);
    QString rootPathAbs = rootDir.path();

    // Fill root nodes

    std::vector<std::string> folders;
    std::vector<DeviceManager::TFileInfoEx> files;

    QString search_path = rootPathAbs + "/*";
    quint32 ret = m_fso->dir(search_path.toStdString().c_str(), folders, files);
    handleError(ret);

    if(ret != ADSERR_NOERR){
        QString errStr = QString("Specified root folder (%1) not found on target").arg(m_rootPath);
        throw std::system_error(ENOENT, std::generic_category(), errStr.toStdString());
    }

    for(const auto& folder : folders){
        auto subFolderNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(rootPathAbs,  QString(folder.c_str()), FileType::Folder, 0, m_fso));
        m_root.push_back(subFolderNode);
    }

    for(const auto& file : files){
        auto fileNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(rootPathAbs, QString(file.fName.c_str()), FileType::File, file.filesize, m_fso));
        m_root.push_back(fileNode);
    }
}

void AdsFileSystemModel::downloadFile(QModelIndex idx)
{
    AdsFileInfoNode* node = reinterpret_cast<AdsFileInfoNode*>(idx.internalPointer());
    if(node && node->m_type == FileType::File){
        emit download(node->m_absPath);
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
    return AmsNetId{ b_netId[0], b_netId[1], b_netId[2], b_netId[3], b_netId[4], b_netId[5] };
}


std::shared_ptr<AdsFileInfoNode> AdsFileSystemModel::getPtr(const AdsFileInfoNode* rawPtr) const
{
    std::shared_ptr<AdsFileInfoNode> ptr;

    if(rawPtr->m_parent){
        for(auto const &p : qAsConst(rawPtr->m_parent->m_children)){
            if(p.get() == rawPtr){
                ptr = p;
                break;
            }
        }
    } else {
        for(auto const &p : qAsConst(m_root)){
            if(p.get() == rawPtr){
                ptr = p;
                break;
            }
        }
    }

    return ptr;
}

int AdsFileSystemModel::rowCount(const QModelIndex &parent) const
{
    std::shared_ptr<AdsFileInfoNode> node;

    if(parent.isValid()){
        node = getPtr(reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer()));
    } else {
        return m_root.count();
    }


    if (node->m_type == FileType::Folder){

        std::vector<std::string> folders;
        std::vector<DeviceManager::TFileInfoEx> files;

        QString search_path = node->m_absPath + "/*";
        qint32 ret = m_fso->dir(search_path.toStdString().c_str(), folders, files);
        qint32 _ret = ret & 0xFFFFFFFF;

        if(_ret == 0xECA70002){
            //node->m_extraInfo = QStringLiteral("Empty");
        } else if (_ret == 0xECA70005) {
            node->m_extraInfo = QStringLiteral("Access denied");
        } else {
            handleError(ret);
        }

        node->m_type = FileType::Folder_Initialized;

        for(const auto& folder : folders){
            auto subFolderNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(node->m_absPath, QString(folder.c_str()), FileType::Folder, 0, m_fso, node ));
            node->m_children.push_back(subFolderNode);
        }

        for(const auto& file : files){
            auto fileNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(node->m_absPath, QString(file.fName.c_str()), FileType::File, file.filesize, m_fso, node ));
            node->m_children.push_back(fileNode);
        }
    }

    return node->m_children.count();
}

int AdsFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QModelIndex AdsFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    AdsFileInfoNode* _node = Q_NULLPTR;
    std::shared_ptr<AdsFileInfoNode> node;

    if(parent.isValid()){
        _node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
        node = getPtr(_node);
        if(node->m_children.count() > row){
            node = node->m_children[row];
            return createIndex(row, column, node.get());
        } else {
            return QModelIndex();
        }
    } else {
        node = m_root[row];
        return createIndex(row, column, node.get());
    }
}

QModelIndex AdsFileSystemModel::parent(const QModelIndex &index) const
{
    AdsFileInfoNode* node = Q_NULLPTR;
    if(!index.isValid()){
        return QModelIndex();
    }

    node = reinterpret_cast<AdsFileInfoNode*>(index.internalPointer());

    if(!node->m_parent){
        return QModelIndex();
    } else {

        std::shared_ptr<AdsFileInfoNode> parent = node->m_parent;

        for(int row = 0; row < parent->m_children.count(); row++){
            if(parent->m_children[row].get() == node){
                return createIndex(row, 0, parent.get());
                break;
            }
        }
    }
    return QModelIndex();
}

QVariant AdsFileSystemModel::data(const QModelIndex &index, int role) const
{
    AdsFileInfoNode* node = Q_NULLPTR;
    if(index.isValid()){
        node = reinterpret_cast<AdsFileInfoNode*>(index.internalPointer());
    } else {
        return QVariant();
    }

    if(role == 0){

        bool isFolder = node->m_type == FileType::Folder || node->m_type == FileType::Folder_Initialized;

        if(index.column() == 0 && !isFolder){
            return QVariant(node->m_name.toString());
        } else if(index.column() == 0 && isFolder){
            return QVariant(QString(node->m_name.toString() + "/"));
        } else if (index.column() == 1 && !isFolder){
            return QVariant(QLocale::system().formattedDataSize(node->m_fileSize));
        } else if (index.column() == 1 && isFolder && node->m_extraInfo.isEmpty()){
            return QVariant("-/-");
        } else if (index.column() == 1 && isFolder){
            return QVariant(node->m_extraInfo);
        } else {
            return QVariant();
        }

    } else {
        return QVariant();
    }
}

QVariant AdsFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    QVariant ret;
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        if(section == 0){
            ret = QVariant(m_rootPath);
        } else if (section == 1) {
            ret = QVariant("Size");
        }
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
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
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

    std::shared_ptr<AdsFileInfoNode> node;
    QModelIndex _parent = parent;

    if(_parent.isValid()){
        node = getPtr(reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer()));
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

            QString targetFile = node->m_absPath + "/" + url.fileName();
            QString localFile = url.toLocalFile();

            size_t size = m_processUpload(localFile, targetFile);

            auto ftest = [&](std::shared_ptr<AdsFileInfoNode> in){ return in->m_absPath == targetFile; };
            bool nodeAlreadyExist = std::find_if(node->m_children.begin(), node->m_children.end(), ftest) != node->m_children.end();

            if(!nodeAlreadyExist){

                //int target_row = (node->m_children.count() == 0) ? 0 : node->m_children.count() - 1;
                int target_row = node->m_children.count();
                beginInsertRows(_parent, target_row, target_row);
                auto newNode = std::shared_ptr<AdsFileInfoNode>(new AdsFileInfoNode(node->m_absPath, url.fileName(), FileType::File, size, m_fso, node));
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

    AdsFileInfoNode* node = reinterpret_cast<AdsFileInfoNode*>(parent.internalPointer());
    if(!node) return false;

    std::shared_ptr<AdsFileInfoNode> parentNode = node->m_parent;

    if(!parentNode) return false;

    quint32 ret = m_fso->deleteFile(node->m_absPath.toStdString().c_str());
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
