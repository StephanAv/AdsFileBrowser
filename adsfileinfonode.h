#ifndef ADSFILEINFONODE_H
#define ADSFILEINFONODE_H

#include <QString>
#include <QStringRef>
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
    AdsFileInfoNode(QString path, QString name, FileType type, qint64 fileSize, std::shared_ptr<DeviceManager::FileSystemObject> fso, std::shared_ptr<AdsFileInfoNode> parent = nullptr);
    ~AdsFileInfoNode();

    QStringRef                                          m_path;
    QStringRef                                          m_name;
    QString                                             m_absPath;
    QString                                             m_extraInfo; // e.g. folder empty, access denied
    FileType                                            m_type;
    qint64                                              m_fileSize;
    std::shared_ptr<DeviceManager::FileSystemObject>    m_fso;
    std::shared_ptr<AdsFileInfoNode>                    m_parent;
    QVector<std::shared_ptr<AdsFileInfoNode>>           m_children;
};

#endif // ADSFILEINFONODE_H
