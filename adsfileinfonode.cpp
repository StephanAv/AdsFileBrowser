#include "adsfileinfonode.h"
#include <QString>
#include <QDebug>

AdsFileInfoNode::AdsFileInfoNode(QString path, FileType type, qint64 fileSize, std::shared_ptr<DeviceManager::FileSystemObject> fso, std::shared_ptr<AdsFileInfoNode> parent)
    : m_path(path)
    , m_type(type)
    , m_fileSize(fileSize)
    , m_fso(fso)
    , m_parent(parent)
{
}

AdsFileInfoNode::~AdsFileInfoNode()
{
    //qDebug() << __func__ << " - " << m_path;
}
