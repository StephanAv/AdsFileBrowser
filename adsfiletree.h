#ifndef ADSFILETREE_H
#define ADSFILETREE_H

#include <QTreeView>

class AdsFileTree : public QTreeView
{
    Q_OBJECT
public:
    AdsFileTree();

signals:
    void download(QModelIndex idx);

public slots:
    void showContextMenu(const QPoint &);
    void deleteFile();
    void downloadSelected();
};

#endif // ADSFILETREE_H
