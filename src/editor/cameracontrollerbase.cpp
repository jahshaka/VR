#include "cameracontrollerbase.h"


void CameraControllerBase::setCamera(QSharedPointer<jah3d::CameraNode>  cam)
{

}

void CameraControllerBase::onMouseDown(Qt::MouseButton button)
{
    switch(button)
    {
        case Qt::LeftButton:
            leftMouseDown = true;
        break;

        case Qt::MiddleButton:
            middleMouseDown = true;
        break;

        case Qt::RightButton:
            rightMouseDown = true;
        break;

    default:
        break;
    }
}

void CameraControllerBase::onMouseUp(Qt::MouseButton button)
{
    switch(button)
    {
        case Qt::LeftButton:
            leftMouseDown = false;
            break;

        case Qt::MiddleButton:
            middleMouseDown = false;
            break;

        case Qt::RightButton:
            rightMouseDown = false;
            break;

        default:
            break;
    }
}

/**
 *
 * issue: when the middle mouse button is down, mouse move events arent registered
 * https://github.com/bjorn/tiled/issues/1079
 * https://bugreports.qt.io/browse/QTBUG-48361
 */
void CameraControllerBase::onMouseMove(int x,int y)
{

}

void CameraControllerBase::onMouseWheel(int val)
{

}

void CameraControllerBase::resetMouseStates()
{
    leftMouseDown = false;
    middleMouseDown = false;
    rightMouseDown = false;
}
