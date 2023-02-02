#ifndef ADSFILEINFONODE_H
#define ADSFILEINFONODE_H

#include <QString>
#include <QVector>

#include "file_system_object.h"

enum FileType {
    Root,
    File,
    Folder,
    Folder_Initialized
};

class AdsFileInfoNode
{
public:
    AdsFileInfoNode(QString path, FileType type, qint64 fileSize, std::shared_ptr<DeviceManager::FileSystemObject> fso, std::shared_ptr<AdsFileInfoNode> parent = nullptr);
    ~AdsFileInfoNode();

    QString                                             m_path;
    QString                                             m_extraInfo; // e.g. folder empty, access denied
    FileType                                            m_type;
    qint64                                              m_fileSize;
    std::shared_ptr<DeviceManager::FileSystemObject>    m_fso;
    std::shared_ptr<AdsFileInfoNode>                    m_parent;
    QVector<std::shared_ptr<AdsFileInfoNode>>           m_children;
};

#endif // ADSFILEINFONODE_H
