#include "adsfiletree.h"
#include <QDebug>
#include <QPoint>
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QModelIndexList>
#include <QMenu>

#include "adsfilesystemmodel.h"

AdsFileTree::AdsFileTree() : QTreeView()
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &AdsFileTree::customContextMenuRequested,
            this, &AdsFileTree::showContextMenu);
}

void AdsFileTree::deleteFile()
{
    QModelIndexList selected = selectedIndexes();
    for(const auto& idx : selected){
        if(!idx.isValid()) continue;

        //AdsFileInfoNode* node = reinterpret_cast<AdsFileInfoNode*>(idx.internalPointer());

        //if(!node) continue;
        model()->removeRows(idx.row(), 1, idx);
        //rowsAboutToBeRemoved(idx, idx.row(), idx.row());


    }
}

void AdsFileTree::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);

    QAction action1(QStringLiteral("Delete File"), this);
    connect(&action1, &QAction::triggered, this, &AdsFileTree::deleteFile);
    contextMenu.addAction(&action1);
    contextMenu.exec(mapToGlobal(pos));
}

void AdsFileTree::downloadSelected(){

    QModelIndexList selected = selectedIndexes();
    //const AdsFileSystemModel* adsModel = reinterpret_cast<AdsFileSystemModel*>(model());

    if(!selected.isEmpty()){
        emit download(selected.first());
        //adsModel->downloadFile();
    }
}
