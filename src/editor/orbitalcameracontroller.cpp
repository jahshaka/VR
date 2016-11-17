#include <QPoint>
#include <QVector3D>
#include <QSharedPointer>
#include <QtMath>

#include "../jah3d/core/scenenode.h"
#include "../jah3d/scenegraph/cameranode.h"
#include "cameracontrollerbase.h"
#include "orbitalcameracontroller.h"


OrbitalCameraController::OrbitalCameraController()
{
    distFromPivot = 15;
}

QSharedPointer<jah3d::CameraNode>  OrbitalCameraController::getCamera()
{
    return camera;
}

/**
 * Calculates the pivot location and its yaw and pitch
 */
void OrbitalCameraController::setCamera(QSharedPointer<jah3d::CameraNode>  cam)
{
    this->camera = cam;

    //calculate the location of the pivot
    auto viewVec = cam->rot.rotatedVector(QVector3D(0,0,-1));//default forward is -z
    pivot = cam->pos + (viewVec*distFromPivot);

    float roll;
    cam->rot.getEulerAngles(&pitch,&yaw,&roll);

    this->updateCameraRot();
}

void OrbitalCameraController::onMouseMove(int x,int y)
{
    if(rightMouseDown)
    {
        //rotate camera
        this->yaw += x/10.0f;
        this->pitch += y/10.0f;
    }

    if(middleMouseDown)
    {
        //translate camera
        float dragSpeed = 0.01f;
        auto dir = camera->rot.rotatedVector(QVector3D(x*dragSpeed,-y*dragSpeed,0));
        pivot += dir;
    }

    updateCameraRot();
}

/**
 * Zooms camera in
 */
void OrbitalCameraController::onMouseWheel(int delta)
{
    //qDebug()<<delta;
    auto zoomSpeed = 0.01f;
    distFromPivot += -delta * zoomSpeed;
    //distFromPivot += delta;

    //todo: remove magic numbers
    if(distFromPivot<0)
        distFromPivot = 0;

    updateCameraRot();
}

void OrbitalCameraController::updateCameraRot()
{
    auto rot = QQuaternion::fromEulerAngles(pitch,yaw,0);
    auto localPos = rot.rotatedVector(QVector3D(0,0,1));

    camera->pos = pivot+(localPos*distFromPivot);
    camera->rot = rot;
    camera->update(0);


}
