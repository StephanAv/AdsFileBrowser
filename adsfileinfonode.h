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
    AdsFileInfoNode(QString path, FileType type, qint64 fileSize, std::shared_ptr<DeviceManager::FileSystemObject> fso, AdsFileInfoNode* parent = Q_NULLPTR);
    ~AdsFileInfoNode();

    QString                                             m_path;
    FileType                                            m_type;
    qint64                                              m_fileSize;
    std::shared_ptr<DeviceManager::FileSystemObject>    m_fso;
    AdsFileInfoNode                                     *m_parent;
    QVector<std::shared_ptr<AdsFileInfoNode>>           m_children;
};

#endif // ADSFILEINFONODE_H
