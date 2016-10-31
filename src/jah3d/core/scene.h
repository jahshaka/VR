#ifndef SCENE_H
#define SCENE_H

#include <QList>
#include <QSharedPointer>
#include "scenenode.h"
#include "../scenegraph/cameranode.h"
#include "../scenegraph/meshnode.h"

namespace jah3d
{

class LightNode;
class SceneNode;
class Scene;

typedef QSharedPointer<jah3d::LightNode> LightNodePtr;
//typedef QSharedPointer<SceneNode> SceneNodePtr;
typedef QSharedPointer<jah3d::Scene> ScenePtr;
typedef QSharedPointer<jah3d::SceneNode> SceneNodePtr;

class Scene
{
private:

    //QList<MeshNodePtr> meshes;

public:
    CameraNodePtr camera;
    SceneNodePtr rootNode;
    QList<LightNodePtr> lights;

    Scene();
public:
    static ScenePtr create();

    void update(float dt);
    void render();

    void addNode(SceneNodePtr node);
    void removeNode(SceneNodePtr node);
};

}


#endif // SCENE_H
