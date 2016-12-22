#include "sceneviewwidget.h"
#include <QTimer>

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QtMath>
#include <QDebug>

#include "../irisgl/src/irisgl.h"
#include "../irisgl/src/scenegraph/meshnode.h"
#include "../irisgl/src/scenegraph/cameranode.h"
#include "../irisgl/src/scenegraph/lightnode.h"
#include "../irisgl/src/materials/defaultmaterial.h"
#include "../irisgl/src/graphics/forwardrenderer.h"
#include "../irisgl/src/graphics/mesh.h"
#include "../irisgl/src/geometry/trimesh.h"
#include "../irisgl/src/graphics/texture2d.h"
#include "../irisgl/src/graphics/viewport.h"
#include "../irisgl/src/graphics/utils/fullscreenquad.h"
#include "../irisgl/src/math/intersectionhelper.h"

#include "../editor/cameracontrollerbase.h"
#include "../editor/editorcameracontroller.h"
#include "../editor/orbitalcameracontroller.h"


#include "../editor/translationgizmo.h"

SceneViewWidget::SceneViewWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    dragging = false;

    QSurfaceFormat format;
    format.setDepthBufferSize(32);
    //format.setMajorVersion(3);
    //format.setMinorVersion(3);
    format.setSamples(1);

    setFormat(format);
    setMouseTracking(true);
    //installEventFilter(this);

    viewport = new iris::Viewport();

    //camController = nullptr;
    defaultCam = new EditorCameraController();
    orbitalCam = new OrbitalCameraController();
    camController = defaultCam;
    //camController = orbitalCam;

    editorCam = iris::CameraNode::create();
    editorCam->pos = QVector3D(0,5,13);
    //editorCam->pos = QVector3D(0,0,10);
    editorCam->rot = QQuaternion::fromEulerAngles(-15,0,0);
    camController->setCamera(editorCam);

    viewportMode = ViewportMode::Editor;
}

void SceneViewWidget::initialize()
{
    translationGizmo = new TranslationGizmo;
    translationGizmo->createHandleShader();
}

void SceneViewWidget::setScene(QSharedPointer<iris::Scene> scene)
{
    this->scene = scene;
    scene->setCamera(editorCam);
    renderer->setScene(scene);

    //remove selected scenenode
    selectedNode.reset();
}

void SceneViewWidget::setSelectedNode(QSharedPointer<iris::SceneNode> sceneNode)
{
    selectedNode = sceneNode;
    renderer->setSelectedSceneNode(sceneNode);
}

void SceneViewWidget::clearSelectedNode()
{
    selectedNode.clear();
    renderer->setSelectedSceneNode(selectedNode);
}

void SceneViewWidget::updateScene()
{
    if (!!translationGizmo->lastSelectedNode) {
        translationGizmo->render(renderer->GLA, ViewMatrix, ProjMatrix);
    }
}

void SceneViewWidget::initializeGL()
{
    makeCurrent();

    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    renderer = iris::ForwardRenderer::create(this);

    initialize();
    fsQuad = new iris::FullScreenQuad();

    emit initializeGraphics(this,this);

    auto timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start();
}

void SceneViewWidget::paintGL()
{
    float dt = 1.0f / 60.0f;
    // scene->update(dt);
    renderScene();
}

void SceneViewWidget::renderScene()
{
    glClearColor(.3f, .3f, .3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!!renderer && !!scene) {
        scene->update(1.0f / 60);

        if (viewportMode == ViewportMode::Editor) {
            // @TODO: find a better way to get the MV matrix from our ogl context
            renderer->renderScene(this->context(), viewport, ViewMatrix, ProjMatrix);
        } else {
            renderer->renderSceneVr(this->context(), viewport);
        }

        this->updateScene();
    }
}

void SceneViewWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    viewport->width = width;
    viewport->height = height;
}

bool SceneViewWidget::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();

    if (type == QEvent::MouseMove) {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(evt);
        return false;
    } else if (type == QEvent::MouseButtonPress) {

        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        mousePressEvent(evt);
        return false;
    } else if (type == QEvent::MouseButtonRelease) {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        mouseReleaseEvent(evt);
        return false;
    }

    return QWidget::eventFilter(obj,event);
}

QVector3D SceneViewWidget::calculateMouseRay(const QPointF& pos)
{
    float x = pos.x();
    float y = pos.y();

    // viewport -> NDC
    float mousex = (2.0f * x) / this->viewport->width - 1.0f;
    float mousey = (2.0f * y) / this->viewport->height - 1.0f;
    QVector2D NDC = QVector2D(mousex, -mousey);

    // NDC -> HCC
    QVector4D HCC = QVector4D(NDC, -1.0f, 1.0f);

    // HCC -> View Space
    QMatrix4x4 projection_matrix_inverse = this->editorCam->projMatrix.inverted();
    QVector4D eye_coords = projection_matrix_inverse * HCC;
    QVector4D ray_eye = QVector4D(eye_coords.x(), eye_coords.y(), -1.0f, 0.0f);

    // View Space -> World Space
    QMatrix4x4 view_matrix_inverse = this->editorCam->viewMatrix.inverted();
    QVector4D world_coords = view_matrix_inverse * ray_eye;
    QVector3D final_ray_coords = QVector3D(world_coords);

    return final_ray_coords.normalized();
}

void SceneViewWidget::mouseMoveEvent(QMouseEvent *e)
{
    // ISSUE - only fired when mouse is dragged
    QPointF localPos = e->localPos();
    QPointF dir = localPos - prevMousePos;

    if (e->buttons() == Qt::LeftButton && !!translationGizmo->currentNode)
    {
         QVector3D ray = (this->calculateMouseRay(localPos) * 512 - this->editorCam->pos).normalized();
         float nDotR = -QVector3D::dotProduct(translationGizmo->translatePlaneNormal, ray);

         if (nDotR != 0.0f) {
             float distance = (QVector3D::dotProduct(
                                   translationGizmo->translatePlaneNormal,
                                   this->editorCam->pos) + translationGizmo->translatePlaneD) / nDotR;
             QVector3D Point = ray * distance + this->editorCam->pos;
             QVector3D Offset = Point - translationGizmo->finalHitPoint;

             if (translationGizmo->currentNode->getName() == "axis__x") {
                 Offset = QVector3D(Offset.x(), 0, 0);
             } else if (translationGizmo->currentNode->getName() == "axis__y") {
                 Offset = QVector3D(0, Offset.y(), 0);
             } else if (translationGizmo->currentNode->getName() == "axis__z") {
                 Offset = QVector3D(0, 0, Offset.z());
             }

             translationGizmo->currentNode->pos += Offset;
             translationGizmo->lastSelectedNode->pos += Offset;

             translationGizmo->finalHitPoint = Point;
         }
     }

    if (camController != nullptr) {
        camController->onMouseMove(-dir.x(),-dir.y());
    }

    prevMousePos = localPos;
}

void SceneViewWidget::mousePressEvent(QMouseEvent *e)
{
    prevMousePos = e->localPos();

    if (e->button() == Qt::RightButton) {
        dragging = true;
    }

    if (e->button() == Qt::LeftButton) {
        editorCam->updateCameraMatrices();
        this->doGizmoPicking(e->localPos());
        this->doObjectPicking(e->localPos());
    }

    if (camController != nullptr) {
        camController->onMouseDown(e->button());
    }
}

void SceneViewWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        dragging = false;
    }

    if (camController != nullptr) {
        camController->onMouseUp(e->button());
    }
}

void SceneViewWidget::wheelEvent(QWheelEvent *event)
{
    if (camController != nullptr) {
        camController->onMouseWheel(event->delta());
    }
}

void SceneViewWidget::doObjectPicking(const QPointF& point)
{
    editorCam->updateCameraMatrices();

    auto segStart = this->editorCam->pos;
    auto rayDir = this->calculateMouseRay(point) * 512;
    // auto rayDir = editorCam->calculatePickingDirection(viewport->width,viewport->height,prevMousePos);
    auto segEnd = segStart + rayDir;

    QList<PickingResult> hitList;
    doScenePicking(scene->getRootNode(), segStart, segEnd, hitList);
    doLightPicking(segStart,segEnd,hitList);

    // find the closest hit and emit signal
    if (hitList.size() == 0) {
        emit sceneNodeSelected(iris::SceneNodePtr());//no hits, deselect object
        return;
    }

    if (hitList.size() == 1) {
        translationGizmo->lastSelectedNode = hitList[0].hitNode;
        emit sceneNodeSelected(hitList[0].hitNode);
        return;
    }

    // sort by distance to camera then return the closest
    qSort(hitList.begin(), hitList.end(), [](const PickingResult& a, const PickingResult& b) {
        return a.distanceFromCameraSqrd > b.distanceFromCameraSqrd;
    });

    translationGizmo->lastSelectedNode = hitList.last().hitNode;
    emit sceneNodeSelected(hitList.last().hitNode);
}

void SceneViewWidget::doGizmoPicking(const QPointF& point)
{
    editorCam->updateCameraMatrices();

    auto segStart = this->editorCam->pos;
    auto rayDir = this->calculateMouseRay(point) * 512;
    // auto rayDir = editorCam->calculatePickingDirection(viewport->width,viewport->height,prevMousePos);
    auto segEnd = segStart + rayDir;

    QList<PickingResult> hitList;
    doMeshPicking(translationGizmo->POINTER, segStart, segEnd, hitList);

    if (hitList.size() == 0) {
        translationGizmo->lastSelectedNode = iris::SceneNodePtr();
        translationGizmo->currentNode = iris::SceneNodePtr();
        emit sceneNodeSelected(iris::SceneNodePtr());
        return;
    }

    if (hitList.size() == 1) {
        translationGizmo->finalHitPoint = hitList[0].hitPoint;

        if (hitList[0].hitNode->getName() == "axis__y") {
            translationGizmo->translatePlaneNormal = QVector3D(.0f, 0.f, 1.f);
        } else {
            translationGizmo->translatePlaneNormal = QVector3D(.0f, 1.f, .0f);
        }

        translationGizmo->translatePlaneD = -QVector3D::dotProduct(translationGizmo->translatePlaneNormal,
                                                                   translationGizmo->finalHitPoint);
        translationGizmo->currentNode = hitList[0].hitNode;
    }

    qSort(hitList.begin(), hitList.end(), [](const PickingResult& a, const PickingResult& b) {
        return a.distanceFromCameraSqrd < b.distanceFromCameraSqrd;
    });

    translationGizmo->finalHitPoint = hitList.last().hitPoint;

    if (hitList.last().hitNode->getName() == "axis__y") {
        translationGizmo->translatePlaneNormal = QVector3D(.0f, 0.f, 1.f);
    } else {
        translationGizmo->translatePlaneNormal = QVector3D(.0f, 1.f, .0f);
    }

    translationGizmo->translatePlaneD = -QVector3D::dotProduct(translationGizmo->translatePlaneNormal,
                                                               translationGizmo->finalHitPoint);
    translationGizmo->currentNode = hitList.last().hitNode;
}

void SceneViewWidget::doScenePicking(const QSharedPointer<iris::SceneNode>& sceneNode,
                                     const QVector3D& segStart,
                                     const QVector3D& segEnd,
                                     QList<PickingResult>& hitList)
{
    if (sceneNode->getSceneNodeType() == iris::SceneNodeType::Mesh)
    {
        auto meshNode = sceneNode.staticCast<iris::MeshNode>();
        auto triMesh = meshNode->getMesh()->getTriMesh();

        //transform segment to local space
        auto invTransform = meshNode->globalTransform.inverted();
        auto a = invTransform * segStart;
        auto b = invTransform * segEnd;

        QList<iris::TriangleIntersectionResult> results;
        if (int resultCount = triMesh->getSegmentIntersections(a, b, results)) {
            for (auto triResult : results) {
                // convert hit to world space
                auto hitPoint = meshNode->globalTransform * triResult.hitPoint;

                PickingResult pick;
                pick.hitNode = sceneNode;
                pick.hitPoint = hitPoint;
                pick.distanceFromCameraSqrd = (hitPoint - editorCam->getGlobalPosition()).lengthSquared();

                hitList.append(pick);
            }
        }
    }

    for (auto child : sceneNode->children) {
        doScenePicking(child, segStart, segEnd, hitList);
    }
}

void SceneViewWidget::doMeshPicking(const QSharedPointer<iris::SceneNode>& sceneNode,
                                    const QVector3D& segStart,
                                    const QVector3D& segEnd,
                                    QList<PickingResult>& hitList)
{
    if (sceneNode->getSceneNodeType() == iris::SceneNodeType::Mesh)
    {
        auto meshNode = sceneNode.staticCast<iris::MeshNode>();
        auto triMesh = meshNode->getMesh()->getTriMesh();

        //transform segment to local space
        auto invTransform = meshNode->globalTransform.inverted();
        auto a = invTransform * segStart;
        auto b = invTransform * segEnd;

        QList<iris::TriangleIntersectionResult> results;
        if (int resultCount = triMesh->getSegmentIntersections(a,b,results)) {
            for (auto triResult : results) {
                // convert hit to world space
                auto hitPoint = meshNode->globalTransform*triResult.hitPoint;

                PickingResult pick;
                pick.hitNode = sceneNode;
                pick.hitPoint = hitPoint;
                pick.distanceFromCameraSqrd = (hitPoint - editorCam->getGlobalPosition()).lengthSquared();

                hitList.append(pick);
            }
        }
    }

    for(auto child:sceneNode->children)
    {
        doScenePicking(child,segStart,segEnd,hitList);
    }
}

void SceneViewWidget::doLightPicking(const QVector3D& segStart,const QVector3D& segEnd,QList<PickingResult>& hitList)
{
    const float lightRadius = 0.5f;

    auto rayDir = (segEnd-segStart);
    float segLengthSqrd = rayDir.lengthSquared();
    rayDir.normalize();
    QVector3D hitPoint;
    float t;

    for(auto light:scene->lights)
    {
        if(iris::IntersectionHelper::raySphereIntersects(segStart,rayDir,light->pos,lightRadius,t,hitPoint))
        {
            PickingResult pick;
            pick.hitNode = light.staticCast<iris::SceneNode>();
            pick.hitPoint = hitPoint;
            pick.distanceFromCameraSqrd = (hitPoint-editorCam->getGlobalPosition()).lengthSquared();

            hitList.append(pick);
        }
    for (auto child : sceneNode->children) {
        doMeshPicking(child, segStart, segEnd, hitList);
    }
}

void SceneViewWidget::setFreeCameraMode()
{
    camController = defaultCam;
    camController->setCamera(editorCam);
    camController->resetMouseStates();
}

void SceneViewWidget::setArcBallCameraMode()
{
    camController = orbitalCam;
    camController->setCamera(editorCam);
    camController->resetMouseStates();
}

bool SceneViewWidget::isVrSupported()
{
    return renderer->isVrSupported();
}

void SceneViewWidget::setViewportMode(ViewportMode viewportMode)
{
    this->viewportMode = viewportMode;
}

ViewportMode SceneViewWidget::getViewportMode()
{
    return viewportMode;
}
