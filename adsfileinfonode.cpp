#include "adsfileinfonode.h"
#include <QString>
#include <QDebug>

AdsFileInfoNode::AdsFileInfoNode(QString path, QString name, FileType type, qint64 fileSize, std::shared_ptr<DeviceManager::FileSystemObject> fso, std::shared_ptr<AdsFileInfoNode> parent)
    : m_absPath(path + "/" + name)
    , m_type(type)
    , m_fileSize(fileSize)
    , m_fso(fso)
    , m_parent(parent)
{

    m_path = QStringRef(&m_absPath, 0, path.length());
    m_name = QStringRef(&m_absPath, path.length() + 1, name.length()); // +1 for the "/"


    int x = 3;
}

AdsFileInfoNode::~AdsFileInfoNode()
{
    //qDebug() << __func__ << " - " << m_path;
}
