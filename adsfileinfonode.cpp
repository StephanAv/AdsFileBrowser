#include "adsfileinfonode.h"
#include <QString>
#include <QDebug>

AdsFileInfoNode::AdsFileInfoNode(QString path, FileType type, std::shared_ptr<DeviceManager::FileSystemObject> fso, AdsFileInfoNode* parent)
    : m_path(path)
    , m_type(type)
    , m_fso(fso)
    , m_parent(parent)
{
}

AdsFileInfoNode::~AdsFileInfoNode()
{
}
