/**************************************************************************
This file is part of JahshakaVR, VR Authoring Toolkit
http://www.jahshaka.com
Copyright (c) 2016  GPLv3 Jahshaka LLC <coders@jahshaka.com>

This is free software: you may copy, redistribute
and/or modify it under the terms of the GPLv3 License

For more information see the LICENSE file
*************************************************************************/

#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include "ui_newmainwindow.h"

#include <qwindow.h>
#include <qsurface.h>

#include <QOpenGLContext>
#include <qstandarditemmodel.h>
#include <QKeyEvent>
#include <QMessageBox>

#include <QFileDialog>

#include <QTreeWidgetItem>

#include <QTimer>
#include <math.h>
#include <QDesktopServices>

#include "dialogs/loadmeshdialog.h"
#include "core/surfaceview.h"
#include "core/nodekeyframeanimation.h"
#include "core/nodekeyframe.h"
#include "globals.h"

#include "io/scenewriter.h"
#include "io/scenereader.h"

#include "widgets/animationwidget.h"

#include "widgets/materialwidget.h"
#include "dialogs/renamelayerdialog.h"
#include "widgets/layertreewidget.h"
#include "core/project.h"
#include "widgets/accordianbladewidget.h"

#include "editor/editorcameracontroller.h"
#include "core/settingsmanager.h"
#include "dialogs/preferencesdialog.h"
#include "dialogs/licensedialog.h"
#include "dialogs/aboutdialog.h"

#include "helpers/collisionhelper.h"

#include "widgets/sceneviewwidget.h"
#include "jah3d/jah3d.h"
#include "jah3d/scenegraph/meshnode.h"
#include "jah3d/scenegraph/cameranode.h"
#include "jah3d/scenegraph/lightnode.h"
#include "jah3d/materials/defaultmaterial.h"
#include "jah3d/graphics/forwardrenderer.h"
#include "jah3d/graphics/mesh.h"
#include "jah3d/graphics/texture2d.h"
#include "jah3d/graphics/viewport.h"

enum class VRButtonMode:int
{
    Default=0,
    Disabled=1,
    VRMode=2
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    //ui(new Ui::MainWindow)
    ui(new Ui::NewMainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Jahshaka VR");

    settings = SettingsManager::getDefaultManager();
    prefsDialog = new PreferencesDialog(settings);
    aboutDialog = new AboutDialog();
    licenseDialog = new LicenseDialog();

    ui->mainTimeline->setMainWindow(this);
    ui->modelpresets->setMainWindow(this);

    camControl = nullptr;
    vrMode = false;

    setupFileMenu();
    setupViewMenu();
    setupHelpMenu();

    this->setupLayerButtonMenu();

    //TIMER
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateAnim()));

    //activeSceneNode = nullptr;

    //this->rebuildTree();
    setProjectTitle(Globals::project->getProjectName());

    //init scene view
    sceneView = new SceneViewWidget(this);
    sceneView->setParent(this);

    //
    QGridLayout* layout = new QGridLayout(ui->sceneContainer);
    layout->addWidget(sceneView);
    layout->setMargin(0);
    ui->sceneContainer->setLayout(layout);
    connect(sceneView,SIGNAL(initializeGraphics(SceneViewWidget*,QOpenGLFunctions_3_2_Core*)),this,SLOT(initializeGraphics(SceneViewWidget*,QOpenGLFunctions_3_2_Core*)));

    //createTestScene();

    ui->sceneHierarchy->setMainWindow(this);
    connect(ui->sceneHierarchy,SIGNAL(sceneNodeSelected(QSharedPointer<jah3d::SceneNode>)),this,SLOT(sceneNodeSelected(QSharedPointer<jah3d::SceneNode>)));
    connect(sceneView,SIGNAL(sceneNodeSelected(QSharedPointer<jah3d::SceneNode>)),this,SLOT(sceneNodeSelected(QSharedPointer<jah3d::SceneNode>)));

    connect(ui->cameraTypeCombo,SIGNAL(currentTextChanged(QString)),this,SLOT(cameraTypeChanged(QString)));
}

void MainWindow::setupVrUi()
{
    ui->vrBtn->setToolTipDuration(0);
    if(sceneView->isVrSupported())
    {
        ui->vrBtn->setEnabled(true);
        ui->vrBtn->setToolTip("Press to view the scene in vr");
        ui->vrBtn->setProperty("vrMode",(int)VRButtonMode::Default);
    }
    else
    {
        ui->vrBtn->setEnabled(false);
        ui->vrBtn->setToolTip("No Oculus device detected");
        ui->vrBtn->setProperty("vrMode",(int)VRButtonMode::Disabled);
    }

    connect(ui->vrBtn,SIGNAL(clicked(bool)),SLOT(vrButtonClicked(bool)));

    //needed to apply changes
    ui->vrBtn->style()->unpolish(ui->vrBtn);
    ui->vrBtn->style()->polish(ui->vrBtn);
}

/**
 * uses style property trick
 * http://wiki.qt.io/Dynamic_Properties_and_Stylesheets
 */
void MainWindow::vrButtonClicked(bool)
{
    if(!sceneView->isVrSupported())
    {
    }
    else
    {

        if(sceneView->getViewportMode()==ViewportMode::Editor)
        {
            sceneView->setViewportMode(ViewportMode::VR);

            //highlight button blue
            ui->vrBtn->setProperty("vrMode",(int)VRButtonMode::VRMode);
        }
        else

        {
            sceneView->setViewportMode(ViewportMode::Editor);

            //return button back to normal color
            ui->vrBtn->setProperty("vrMode",(int)VRButtonMode::Default);
        }
    }

    //needed to apply changes
    ui->vrBtn->style()->unpolish(ui->vrBtn);
    ui->vrBtn->style()->polish(ui->vrBtn);
}

//create test scene
void MainWindow::initializeGraphics(SceneViewWidget* widget,QOpenGLFunctions_3_2_Core* gl)
{
    auto scene = jah3d::Scene::create();

    auto cam = jah3d::CameraNode::create();
    cam->pos = QVector3D(0,15,10);
    cam->rot = QQuaternion::fromEulerAngles(-60,0,0);
    //cam->lookAt(QVector3D(0,0,0),QVect);

    scene->setCamera(cam);
    //camControl = new EditorCameraController(cam);
    //scene->rootNode->addChild(cam);//editor camera shouldnt be a part of the scene itself

    //second node
    auto node = jah3d::MeshNode::create();
    //boxNode->setMesh("app/models/head.obj");
    node->setMesh("app/models/plane.obj");
    node->scale = QVector3D(100,1,100);
    node->setName("Ground");

    auto m = jah3d::DefaultMaterial::create();
    node->setMaterial(m);
    m->setDiffuseColor(QColor(255,255,255));
    m->setDiffuseTexture(jah3d::Texture2D::load("app/content/textures/defaultgrid.png"));
    m->setShininess(0);
    m->setTextureScale(100);
    scene->rootNode->addChild(node);


    //add test object with basic material
    auto boxNode = jah3d::MeshNode::create();
    //boxNode->setMesh("app/models/head.obj");
    //boxNode->setMesh("app/models/box.obj");
    boxNode->setMesh("assets/models/StanfordDragon.obj");

    auto mat = jah3d::DefaultMaterial::create();
    boxNode->setMaterial(mat);
    mat->setDiffuseColor(QColor(255,200,200));
    mat->setDiffuseTexture(jah3d::Texture2D::load("app/content/textures/Artistic Pattern.png"));


    //lighting
    auto light = jah3d::LightNode::create();
    light->setLightType(jah3d::LightType::Point);
    light->rot = QQuaternion::fromEulerAngles(45,0,0);
    scene->rootNode->addChild(light);
    //light->pos = QVector3D(5,5,0);
    light->setName("Light");
    light->pos = QVector3D(-5,5,3);
    light->intensity = 1;
    light->icon = jah3d::Texture2D::load("app/icons/bulb.png");


    scene->rootNode->addChild(boxNode);

    //sceneView->setScene(scene);
    this->setScene(scene);

    setupVrUi();
}

void MainWindow::cameraTypeChanged(QString type)
{
    if(type=="Free")
    {
        sceneView->setFreeCameraMode();
    }
    else
    {
        sceneView->setArcBallCameraMode();
    }
}

void MainWindow::setSettingsManager(SettingsManager* settings)
{
    this->settings = settings;
}

SettingsManager* MainWindow::getSettingsManager()
{
    return settings;
}

bool MainWindow::handleMousePress(QMouseEvent *event)
{
    mouseButton = event->button();
    mousePressPos = event->pos();

    //if(event->button() == Qt::LeftButton)
    //    gizmo->onMousePress(event->x(),event->y());

    return true;
}

bool MainWindow::handleMouseRelease(QMouseEvent *event)
{
    //if(activeGizmoHandle!=nullptr)
    //    activeGizmoHandle->removeHighlight();

    /*
    mouseButton = event->button();
    mouseReleasePos = event->pos();



    //show context menu
    if(mouseButton == Qt::RightButton
            && (mouseReleasePos - mousePressPos).manhattanLength() < 3 //mouse press should be equal to mouse release
            && activeSceneNode != nullptr
            )
    {
        QMenu menu;
        createSceneNodeContextMenu(menu,activeSceneNode);

        menu.exec(mouseReleasePos);
    }
    else if(mouseButton == Qt::LeftButton)
    {
        gizmo->onMouseRelease(event->x(),event->y());
    }
    */
    return true;
}

bool MainWindow::handleMouseMove(QMouseEvent *event)
{
    mousePos = event->pos();

    //gizmo->onMouseMove(event->x(),event->y());

    return false;
}

//todo: disable scrolling while doing gizmo transform
bool MainWindow::handleMouseWheel(QWheelEvent *event)
{
    /*
    auto speed = 0.01f;
    this->camControl->getCamera()->translate(QVector3D(0,0,event->delta()*speed));
    */
    return false;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    switch (event->type()) {
    case QEvent::KeyPress: {
        break;
    }
    case QEvent::MouseButtonPress:
        if (obj == surface)
            return handleMousePress(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonRelease:
        if (obj == surface)
            return handleMouseRelease(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseMove:
        if (obj == surface)
            return handleMouseMove(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::Wheel:
        if (obj == surface)
            return handleMouseWheel(static_cast<QWheelEvent *>(event));
        break;
    default:
        break;
    }

    return false;
}

void MainWindow::setupFileMenu()
{
    //add recent files
    auto recent = settings->getRecentlyOpenedScenes();
    if(recent.size()==0)
    {
        ui->menuOpenRecent->setEnabled(false);
    }
    else
    {
        ui->menuOpenRecent->setEnabled(true);

        ui->menuOpenRecent->clear();
        for(auto item:recent)
        {
            //if(item.size()>20)
            //    item = item;
            auto action = new QAction(item,ui->menuOpenRecent);
            action->setData(item);
            connect(action,SIGNAL(triggered(bool)),this,SLOT(openRecentFile()));
            ui->menuOpenRecent->addAction(action);
        }
    }


    connect(ui->actionSave,SIGNAL(triggered(bool)),this,SLOT(saveScene()));
    connect(ui->actionSave_As,SIGNAL(triggered(bool)),this,SLOT(saveSceneAs()));
    connect(ui->actionLoad,SIGNAL(triggered(bool)),this,SLOT(loadScene()));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(exitApp()));
    connect(ui->actionPreferences,SIGNAL(triggered(bool)),this,SLOT(showPreferences()));
    connect(ui->actionNew,SIGNAL(triggered(bool)),this,SLOT(newScene()));

}

void MainWindow::setupViewMenu()
{
    connect(ui->actionEditorCamera,SIGNAL(triggered(bool)),this,SLOT(useEditorCamera()));
    connect(ui->actionViewerCamera,SIGNAL(triggered(bool)),this,SLOT(useUserCamera()));
}

void MainWindow::setupHelpMenu()
{
    connect(ui->actionLicense,SIGNAL(triggered(bool)),this,SLOT(showLicenseDialog()));
    connect(ui->actionAbout,SIGNAL(triggered(bool)),this,SLOT(showAboutDialog()));
    connect(ui->actionBlog,SIGNAL(triggered(bool)),this,SLOT(openBlogUrl()));
    connect(ui->actionVisit_Website,SIGNAL(triggered(bool)),this,SLOT(openWebsiteUrl()));
}

void MainWindow::setProjectTitle(QString projectTitle)
{
    this->setWindowTitle(projectTitle+" - Jahshaka");
}

void MainWindow::sceneTreeCustomContextMenu(const QPoint& pos)
{
    /*
    QModelIndex index = ui->sceneTree->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    auto item = ui->sceneTree->itemAt(pos);
    auto node = (SceneNode*)item->data(1,Qt::UserRole).value<void*>();
    QMenu menu;
    QAction* action;

    //rename
    action = new QAction(QIcon(),"Rename",this);
    connect(action,SIGNAL(triggered()),this,SLOT(renameNode()));
    menu.addAction(action);
    */

    /*
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
    */

    //menu.exec(ui->sceneTree->mapToGlobal(pos));
}

void MainWindow::renameNode()
{
    RenameLayerDialog dialog(this);
    dialog.setName(activeSceneNode->getName());
    dialog.exec();

    activeSceneNode->setName(dialog.getName());
    //item->setText(0,node->getName());

    //for now
    this->ui->sceneHierarchy->repopulateTree();

}

void MainWindow::updateGizmoTransform()
{
    //if(activeSceneNode==nullptr)
    //    return;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    float aspectRatio = ui->sceneContainer->width()/(float)ui->sceneContainer->height();
    //editorCam->lens()->setPerspectiveProjection(45.0f, aspectRatio, 0.1f, 1000.0f);
}

void MainWindow::stopAnimWidget()
{
    animWidget->stopAnimation();
}
/*
void MainWindow::useEditorCamera()
{
    forwardRenderer->setCamera(editorCam);

    auto cam = (UserCameraNode*)this->scene->getRootNode()->children[0];
    cam->showBody();
}
*/
/*
void MainWindow::useUserCamera()
{
    auto cam = (UserCameraNode*)this->scene->getRootNode()->children[0];
    forwardRenderer->setCamera(editorCam);
    cam->hideBody();
}
*/

void MainWindow::saveScene()
{

    if(Globals::project->isSaved())
    {
        auto filename = Globals::project->getFilePath();
        auto writer = new SceneWriter();
        writer->writeScene(filename,scene);

        settings->addRecentlyOpenedScene(filename);

        delete writer;
    }
    else
    {
        auto filename = QFileDialog::getSaveFileName(this,"Save Scene","","Jashaka Scene (*.jah)");
        auto writer = new SceneWriter();
        writer->writeScene(filename,scene);

        Globals::project->setFilePath(filename);
        this->setProjectTitle(Globals::project->getProjectName());

        settings->addRecentlyOpenedScene(filename);

        delete writer;
    }

}

void MainWindow::saveSceneAs()
{

    QString dir = QApplication::applicationDirPath()+"/scenes/";
    auto filename = QFileDialog::getSaveFileName(this,"Save Scene",dir,"Jashaka Scene (*.jah)");
    auto writer = new SceneWriter();
    writer->writeScene(filename,scene);

    Globals::project->setFilePath(filename);
    this->setProjectTitle(Globals::project->getProjectName());

    settings->addRecentlyOpenedScene(filename);

    delete writer;
}

void MainWindow::loadScene()
{
    QString dir = QApplication::applicationDirPath()+"/scenes/";
    auto filename = QFileDialog::getOpenFileName(this,"Open Scene File",dir,"Jashaka Scene (*.jah)");

    if(filename.isEmpty() || filename.isNull())
        return;

    openProject(filename);
}

void MainWindow::openProject(QString filename)
{

    //remove current scene first
    this->removeScene();

    //load new scene
    auto reader = new SceneReader();

    auto scene = reader->readScene(filename);
    //auto sceneEnt = new Qt3DCore::QEntity();
    //sceneEnt->setParent(rootEntity);
    //auto scene = exp->loadScene(filename,sceneEnt);
    setScene(scene);

    Globals::project->setFilePath(filename);
    this->setProjectTitle(Globals::project->getProjectName());

    settings->addRecentlyOpenedScene(filename);

    delete reader;

}

/**
 * @brief callback for "Recent Files" submenu actions
 * opens files directly, no file dialog
 * shows dialog box showing error if file doesnt exist anymore
 * @todo request if file should be saved
 * @param action
 */
void MainWindow::openRecentFile()
{
    auto action = qobject_cast<QAction*>(sender());
    auto filename = action->data().toString();
    QFileInfo info(filename);

    if(!info.exists())
    {
        QMessageBox box(this);
        box.setText("unable not locate file '"+filename+"'");
        box.exec();
        return;
    }

    //auto exp = new SceneParser();
    //auto sceneEnt = new Qt3DCore::QEntity();
    //sceneEnt->setParent(rootEntity);

    //auto scene = exp->loadScene(filename,sceneEnt);
    //setScene(scene);

    Globals::project->setFilePath(filename);
    this->setProjectTitle(Globals::project->getProjectName());

    settings->addRecentlyOpenedScene(filename);
}

void MainWindow::setScene(QSharedPointer<jah3d::Scene> scene)
{
    this->scene = scene;
    this->sceneView->setScene(scene);
    ui->sceneHierarchy->setScene(scene);
}

void MainWindow::removeScene()
{
}

void MainWindow::setupPropertyUi()
{
    //TRANSFROMS
    //transformUi = new TransformWidget();

    //LIGHT LAYER
    //lightLayerWidget = new LightLayerWidget();

    //MATERIAL UI
    materialWidget = new MaterialWidget();

    //ANIMATION
    animWidget = new AnimationWidget();
    animWidget->setMainTimelineWidget(ui->mainTimeline->getTimeline());

    //WORLD
    //worldLayerWidget = new WorldLayerWidget();
}

void MainWindow::sceneNodeSelected(QTreeWidgetItem* item)
{
    //auto data = (SceneNode*)item->data(1,Qt::UserRole).value<void*>();
    //setActiveSceneNode(data);
}

void MainWindow::sceneTreeItemChanged(QTreeWidgetItem* item,int column)
{
    /*
    auto node = (SceneNode*)item->data(1,Qt::UserRole).value<void*>();

    if(item->checkState(column) == Qt::Checked)
    {
        node->show();
    }
    else
    {
        node->hide();
    }
    */
}

void MainWindow::sceneNodeSelected(QSharedPointer<jah3d::SceneNode> sceneNode)
{
    //todo: ensure this node is a part of the scene
    //scene->setHighlightedNode(sceneNode);

    //show properties for scenenode
    activeSceneNode = sceneNode;

    /*
    //ui->accordian->add
    auto transBlade = new AccordianBladeWidget();
    transBlade->setContentTitle("Transformation");
    transBlade->addTransform();
    transBlade->expand();
    auto etc = new AccordianBladeWidget();
    etc->setContentTitle("title");
    etc->addFilePicker("Mesh:");
    etc->expand();

    auto layout = new QVBoxLayout();
    layout->addWidget(transBlade);
    layout->addWidget(etc);
    layout->addStretch();
    layout->setMargin(0);

    ui->sceneNodeProperties->setLayout(layout);
    */
    sceneView->setSelectedNode(sceneNode);
    ui->sceneNodeProperties->setSceneNode(sceneNode);
    ui->sceneHierarchy->setSelectedNode(sceneNode);
}


void MainWindow::setupLayerButtonMenu()
{
    /*
    QMenu* addMenu = new QMenu();

    QAction *action = new QAction("2D Scene", this);
    addMenu->addAction(action);

    action = new QAction("Scene Graph", this);
    addMenu->addAction(action);
    //connect(action,SIGNAL(triggered()),this,SLOT(addSceneGraphLayer()));

    action = new QAction("Text", this);
    addMenu->addAction(action);

    action = new QAction("Particles", this);
    addMenu->addAction(action);

    action = new QAction("Video", this);
    addMenu->addAction(action);
    */
}

void MainWindow::updateAnim()
{
    //scene->getRootNode()->update(1/60.0f);
    //scene->getRootNode()->applyAnimationAtTime(Globals::animFrameTime);
}

void MainWindow::setSceneAnimTime(float time)
{
    //scene->getRootNode()->applyAnimationAtTime(time);
}

/**
 * Adds a cube to the current scene
 */
void MainWindow::addCube()
{
    auto node = jah3d::MeshNode::create();
    node->setMesh("app/content/primitives/cube.obj");
    node->setName("Cube");

    addNodeToScene(node);
}

/**
 * Adds a torus to the current scene
 */
void MainWindow::addTorus()
{
    auto node = jah3d::MeshNode::create();
    node->setMesh("app/content/primitives/torus.obj");
    node->setName("Torus");

    addNodeToScene(node);
}

/**
 * Adds a sphere to the current scene
 */
void MainWindow::addSphere()
{
    auto node = jah3d::MeshNode::create();
    node->setMesh("app/content/primitives/sphere.obj");
    node->setName("Sphere");

    addNodeToScene(node);
}

/**
 * Adds a cylinder to the current scene
 */
void MainWindow::addCylinder()
{
    auto node = jah3d::MeshNode::create();
    node->setMesh("app/content/primitives/cylinder.obj");
    node->setName("Cylinder");

    addNodeToScene(node);
}

void MainWindow::addPointLight()
{
    auto node = jah3d::LightNode::create();
    node->setLightType(jah3d::LightType::Point);
    node->icon = jah3d::Texture2D::load("app/icons/bulb.png");

    addNodeToScene(node);
}

void MainWindow::addSpotLight()
{
    auto node = jah3d::LightNode::create();
    node->setLightType(jah3d::LightType::Spot);
    node->icon = jah3d::Texture2D::load("app/icons/bulb.png");

    addNodeToScene(node);
}

void MainWindow::addDirectionalLight()
{
    auto node = jah3d::LightNode::create();
    node->setLightType(jah3d::LightType::Directional);
    node->icon = jah3d::Texture2D::load("app/icons/bulb.png");

    addNodeToScene(node);
}

void MainWindow::addMesh()
{
    QString dir = QApplication::applicationDirPath()+"/assets/models/";
    //qDebug()<<dir;
    auto filename = QFileDialog::getOpenFileName(this,"Load Mesh",dir,"Mesh Files (*.obj *.fbx *.3ds)");
    auto nodeName = QFileInfo(filename).baseName();
    if(filename.isEmpty())
        return;


    //auto node = jah3d::MeshNode::create();
    //node->setMesh(filename);
    //node->setName(nodeName);

    auto node = jah3d::MeshNode::loadAsSceneFragment(filename);
    node->setName(nodeName);

    //todo: load material data
    addNodeToScene(node);
}

void MainWindow::addViewPoint()
{
    //addSceneNodeToSelectedTreeItem(ui->sceneTree,new UserCameraNode(new Qt3DCore::QEntity()),true,getIconFromSceneNodeType(SceneNodeType::UserCamera));
}

void MainWindow::addTexturedPlane()
{
    /*
    auto node = TexturedPlaneNode::createTexturedPlane("Textured Plane");
    //node->setTexture(path);
    addSceneNodeToSelectedTreeItem(ui->sceneTree,node,false,QIcon(":app/icons/square.svg"));
    */
}

/**
 * Adds sceneNode to selected scene node. If there is no selected scene node,
 * sceneNode is added to the root node
 * @param sceneNode
 */
void MainWindow::addNodeToActiveNode(QSharedPointer<jah3d::SceneNode> sceneNode)
{
    if(!scene)
    {
        //todo: set alert that a scene needs to be set before this can be done
    }

    //apply default material
    if(sceneNode->sceneNodeType == jah3d::SceneNodeType::Mesh)
    {
        auto meshNode = sceneNode.staticCast<jah3d::MeshNode>();

        if(!meshNode->getMaterial())
        {
            auto mat = jah3d::DefaultMaterial::create();
            meshNode->setMaterial(mat);
        }

    }

    if(!!activeSceneNode)
    {
        activeSceneNode->addChild(sceneNode);
    }
    else
    {
        scene->getRootNode()->addChild(sceneNode);
    }

    ui->sceneHierarchy->repopulateTree();
}

/**
 * adds sceneNode directly to the scene's rootNode
 * applied default material to mesh if one isnt present
 */
void MainWindow::addNodeToScene(QSharedPointer<jah3d::SceneNode> sceneNode)
{
    if(!scene)
    {
        //todo: set alert that a scene needs to be set before this can be done
        return;
    }

    //apply default material
    if(sceneNode->sceneNodeType == jah3d::SceneNodeType::Mesh)
    {
        auto meshNode = sceneNode.staticCast<jah3d::MeshNode>();

        if(!meshNode->getMaterial())
        {
            auto mat = jah3d::DefaultMaterial::create();
            meshNode->setMaterial(mat);
        }

    }

    scene->getRootNode()->addChild(sceneNode);

    ui->sceneHierarchy->repopulateTree();
}

void MainWindow::duplicateNode()
{
    /*
    auto items = ui->sceneTree->selectedItems();
    if(items.size()==0)
        return;
    auto selectedItem = items[0];

    auto node = (SceneNode*)selectedItem->data(1,Qt::UserRole).value<void*>();

    //world node isnt removable
    if(!node->isDuplicable())
        return;

    QTreeWidgetItem* item = nullptr;
    SceneNode* parentNode = nullptr;

    //use rootnode instead
    parentNode = scene->getRootNode();
    //item = ui->sceneTree->invisibleRootItem();
    item = ui->sceneTree->invisibleRootItem()->child(0);

    auto newNode = node->duplicate();
    newNode->setName(node->getName());
    parentNode->addChild(newNode);
    //newNode->pos.setY(3);
    newNode->_updateTransform();
    auto newNodeEnt = newNode->getEntity();

    //tree node
    auto newItem = new QTreeWidgetItem();
    newItem->setText(0,newNode->getName());
    newItem->setData(1,Qt::UserRole,QVariant::fromValue((void*)newNode));
    //newItem->setIcon(0,icon);//this should be changed based on the type of node
    newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
    newItem->setCheckState(0,Qt::Checked);
    item->addChild(newItem);

    ui->sceneTree->expandAll();
    this->setActiveSceneNode(newNode);

    //select item
    items = ui->sceneTree->selectedItems();
    for(auto i:items)
        ui->sceneTree->setItemSelected(i,false);

    ui->sceneTree->setItemSelected(newItem,true);
    //sceneNodes.insert(newNodeEnt->id(),newNode);
    sceneTreeItems.insert(newNodeEnt->id(),newItem);
    */
}

void MainWindow::deleteNode()
{
    if(!!activeSceneNode)
    {
        activeSceneNode->removeFromParent();
        ui->sceneHierarchy->repopulateTree();
        sceneView->clearSelectedNode();
        ui->sceneNodeProperties->setSceneNode(QSharedPointer<jah3d::SceneNode>(nullptr));
    }

    /*
    auto items = ui->sceneTree->selectedItems();
    if(items.size()==0)
        return;
    auto item = items[0];

    auto node = (SceneNode*)item->data(1,Qt::UserRole).value<void*>();

    //world node isnt removable
    if(!node->isRemovable())
        return;

    item->parent()->removeChild(item);
    node->getParent()->removeChild(node);
    //sceneNodes.remove(node->getEntity()->id());
    sceneTreeItems.remove(node->getEntity()->id());

    ui->tabWidget->clear();
    */
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief accepts model files dropped into scene
 * currently only .obj files are supported
 */
void MainWindow::dropEvent(QDropEvent* event)
{
    /*
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size(); i++)
        {
            QString filename = urlList.at(i).toLocalFile();

            //only use .obj files for now
            QFileInfo fileInfo(filename);
            qDebug()<<fileInfo.completeSuffix();
            if(fileInfo.completeSuffix()!="obj")
                continue;

            auto nodeName = fileInfo.baseName();

            auto node = MeshNode::loadMesh(nodeName,filename);

            auto mat = new AdvanceMaterial();
            node->setMaterial(mat);

            addSceneNodeToSelectedTreeItem(ui->sceneTree,node,false,getIconFromSceneNodeType(node->sceneNodeType));
        }

    }

    event->acceptProposedAction();
    */
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}


QIcon MainWindow::getIconFromSceneNodeType(SceneNodeType type)
{
    /*
    switch(type)
    {
        case SceneNodeType::Cube:
        case SceneNodeType::Cylinder:
        case SceneNodeType::Mesh:
        case SceneNodeType::Sphere:
        case SceneNodeType::Torus:
            return QIcon(":app/icons/sceneobject.svg");
        case SceneNodeType::Light:
            return QIcon(":app/icons/light.svg");
        case SceneNodeType::World:
            return QIcon(":app/icons/world.svg");
        case SceneNodeType::UserCamera:
            return QIcon(":app/icons/people.svg");
    default:
        return QIcon();
    }
    */

    return QIcon();
}

void MainWindow::showPreferences()
{
    prefsDialog->exec();
}

void MainWindow::exitApp()
{
    QApplication::exit();
}

void MainWindow::newScene()
{

}

void MainWindow::showAboutDialog()
{
    aboutDialog->exec();
}

void MainWindow::showLicenseDialog()
{
    licenseDialog->exec();
}

void MainWindow::openBlogUrl()
{
    QDesktopServices::openUrl(QUrl("http://www.jahshaka.com/category/blog/"));
}

void MainWindow::openWebsiteUrl()
{
    QDesktopServices::openUrl(QUrl("http://www.jahshaka.com/"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
