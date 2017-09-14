#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

namespace Ui {
    class AssetWidget;
}

#include <QListWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <QFileDialog>

#include "../io/assetmanager.h"
#include "../editor/thumbnailgenerator.h"

// TODO - https://stackoverflow.com/questions/19465812/how-can-i-insert-qdockwidget-as-tab

struct AssetItem {
    QString selectedPath;
    QTreeWidgetItem *item;
    QListWidgetItem *wItem;
    // add one for assetView...
};

#define     ID_ROLE     0x0111

class AssetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetWidget(QWidget *parent = Q_NULLPTR);
    ~AssetWidget();

    void populateAssetTree();
    void updateTree(QTreeWidgetItem* parentTreeItem, QString path);
    void walkFileSystem(QString folder, QString path);
    void addItem(const QString &asset);
    void updateAssetView(const QString &path);
    void trigger();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

protected slots:
    void treeItemSelected(QTreeWidgetItem* item);
    void treeItemChanged(QTreeWidgetItem* item,int index);


    void sceneTreeCustomContextMenu(const QPoint &);
    void sceneViewCustomContextMenu(const QPoint &);
    void assetViewClicked(QListWidgetItem*);
    void assetViewDblClicked(QListWidgetItem*);

    void updateAssetItem();

    void renameTreeItem();
    void renameViewItem();

    void searchAssets(QString);
    void OnLstItemsCommitData(QWidget*);

    void deleteTreeFolder();
    void deleteViewFolder();
    void openAtFolder();
    void createFolder();
    void importAssetB();
    void importAsset(const QStringList &path);

    void onThumbnailResult(ThumbnailResult* result);

private:
    Ui::AssetWidget *ui;
    AssetItem assetItem;
    QPoint startPos;

    QString currentPath;
};

#endif // ASSETWIDGET_H
