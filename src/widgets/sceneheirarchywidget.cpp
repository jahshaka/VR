#include "sceneheirarchywidget.h"
#include "ui_sceneheirarchywidget.h"

#include <QTreeWidgetItem>
#include <QMenu>
#include <QDebug>

#include "../mainwindow.h"

#include "../jah3d/core/scene.h"
#include "../jah3d/core/scenenode.h"

SceneHeirarchyWidget::SceneHeirarchyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneHeirarchyWidget)
{
    ui->setupUi(this);

    mainWindow = nullptr;

    connect(ui->sceneTree,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(treeItemSelected(QTreeWidgetItem*)));
    connect(ui->sceneTree,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(treeItemChanged(QTreeWidgetItem*,int)));

    //make items draggable and droppable
    ui->sceneTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->sceneTree->setDragEnabled(true);
    ui->sceneTree->viewport()->setAcceptDrops(true);
    ui->sceneTree->setDropIndicatorShown(true);
    ui->sceneTree->setDragDropMode(QAbstractItemView::InternalMove);

    //custom context menu
    //http://stackoverflow.com/questions/22198427/adding-a-right-click-menu-for-specific-items-in-qtreeview
    ui->sceneTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sceneTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(sceneTreeCustomContextMenu(const QPoint &)));

}

void SceneHeirarchyWidget::setScene(QSharedPointer<jah3d::Scene> scene)
{
    //todo: cleanly remove previous scene
    this->scene = scene;
    this->repopulateTree();
}

void SceneHeirarchyWidget::setMainWindow(MainWindow* mainWin)
{
    mainWindow = mainWin;

    //ADD BUTTON
    //todo: bind callbacks
    QMenu* addMenu = new QMenu();

    auto primtiveMenu = addMenu->addMenu("Primtives");
    QAction *action = new QAction("Torus", this);
    primtiveMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addTorus()));

    action = new QAction("Cube", this);
    primtiveMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addCube()));

    action = new QAction("Sphere", this);
    primtiveMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addSphere()));

    action = new QAction("Cylinder", this);
    primtiveMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addCylinder()));

    action = new QAction("Plane", this);
    primtiveMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addTexturedPlane()));

    //LIGHTS
    auto lightMenu = addMenu->addMenu("Lights");
    action = new QAction("PointLight", this);
    lightMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addPointLight()));

    action = new QAction("SpotLight", this);
    lightMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addSpotLight()));

    action = new QAction("DirectionalLight", this);
    lightMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addDirectionalLight()));

    //MESHES
    action = new QAction("Load 3D Object", this);
    addMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addMesh()));

    //VIEWPOINT
    action = new QAction("ViewPoint", this);
    addMenu->addAction(action);
    connect(action,SIGNAL(triggered()),mainWindow,SLOT(addViewPoint()));

    ui->addBtn->setMenu(addMenu);
    ui->addBtn->setPopupMode(QToolButton::InstantPopup);
}

void SceneHeirarchyWidget::treeItemSelected(QTreeWidgetItem* item)
{
    //qDebug()<<"tree item selected"<<endl;
    long nodeId = item->data(1,Qt::UserRole).toLongLong();
    selectedNode = nodeList[nodeId];

    emit sceneNodeSelected(selectedNode);
}

void SceneHeirarchyWidget::treeItemChanged(QTreeWidgetItem* item,int column)
{
    //qDebug()<<"tree item changed"<<endl;
    long nodeId = item->data(1,Qt::UserRole).toLongLong();
    auto node = nodeList[nodeId];

    if(item->checkState(column) == Qt::Checked)
    {
        node->show();
        //qDebug()<<"show node"<<endl;
    }
    else
    {
        node->hide();
        //qDebug()<<"hide node"<<endl;
    }

    //qDebug()<<node->getName()+" is "+(node->isVisible()?"visible":"invisible")<<endl;
}

void SceneHeirarchyWidget::sceneTreeCustomContextMenu(const QPoint& pos)
{
    QModelIndex index = ui->sceneTree->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    auto item = ui->sceneTree->itemAt(pos);
    auto nodeId = (long)item->data(1,Qt::UserRole).toLongLong();
    auto node = nodeList[nodeId];

    QMenu menu;
    QAction* action;

    //rename
    action = new QAction(QIcon(),"Rename",this);
    connect(action,SIGNAL(triggered()),this,SLOT(renameNode()));
    menu.addAction(action);

    //world node isnt removable
    if(node->isRemovable())
    {
        action = new QAction(QIcon(),"Delete",this);
        connect(action,SIGNAL(triggered()),this,SLOT(deleteNode()));
        menu.addAction(action);
    }

    if(node->isDuplicable())
    {
        action = new QAction(QIcon(),"Duplicate",this);
        connect(action,SIGNAL(triggered()),this,SLOT(duplicateNode()));
        menu.addAction(action);
    }

    selectedNode = node;
    menu.exec(ui->sceneTree->mapToGlobal(pos));
}

void SceneHeirarchyWidget::renameNode()
{
    //mainWindow->deleteNode();
    mainWindow->renameNode();
}

void SceneHeirarchyWidget::deleteNode()
{
    mainWindow->deleteNode();
    selectedNode.clear();
}

void SceneHeirarchyWidget::duplicateNode()
{

}

void SceneHeirarchyWidget::repopulateTree()
{
    auto rootNode = scene->getRootNode();
    auto root = new QTreeWidgetItem();

    root->setText(0,rootNode->getName());
    //root->setIcon(0,this->getIconFromSceneNodeType(SceneNodeType::World));
    root->setData(1,Qt::UserRole,QVariant::fromValue(rootNode->getNodeId()));

    //populate tree
    nodeList.clear();
    nodeList.insert(rootNode->getNodeId(),rootNode);

    populateTree(root,rootNode);

    ui->sceneTree->clear();
    ui->sceneTree->addTopLevelItem(root);
    ui->sceneTree->expandAll();
}

void SceneHeirarchyWidget::populateTree(QTreeWidgetItem* parentNode,QSharedPointer<jah3d::SceneNode> sceneNode)
{
    for(auto node:sceneNode->children)
    {

        auto childNode = new QTreeWidgetItem();
        childNode->setText(0,node->getName());
        childNode->setData(1,Qt::UserRole,QVariant::fromValue(node->getNodeId()));
        //childNode->setIcon(0,this->getIconFromSceneNodeType(node->sceneNodeType));
        childNode->setFlags(childNode->flags() | Qt::ItemIsUserCheckable);
        childNode->setCheckState(0,Qt::Checked);


        //if(!node->isVisible())
        //    childNode->setCheckState(0,Qt::Unchecked);

        parentNode->addChild(childNode);

        //sceneTreeItems.insert(node->getEntity()->id(),childNode);
        nodeList.insert(node->getNodeId(),node);

        populateTree(childNode,node);
    }
}

/*
void addTorus()
{

}

void addCube()
{

}

void addSphere()
{

}

void addCylinder()
{

}

void addTexturedPlane();
void addPointLight();
void addSpotLight();
void addDirectionalLight();
void addMesh();
void addViewPoint();
*/

SceneHeirarchyWidget::~SceneHeirarchyWidget()
{
    delete ui;
}
