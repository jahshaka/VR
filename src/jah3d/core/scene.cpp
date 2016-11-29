#include "scene.h"
#include "scenenode.h"
#include "../scenegraph/lightnode.h"
#include "../scenegraph/cameranode.h"
#include "../graphics/mesh.h"

namespace jah3d
{

Scene::Scene()
{
    rootNode = SceneNode::create();
    rootNode->setName("Scene");
    //rootNode->setScene(this->sharedFromThis());

    skyMesh = Mesh::loadMesh("app/content/primitives/sky.obj");
    skyTexture = Texture2D::load("app/content/skies/default.png");
}

void Scene::setSkyTexture(Texture2DPtr tex)
{
    skyTexture = tex;
}

QString Scene::getSkyTextureSource()
{
    return skyTexture->getSource();
}

void Scene::clearSkyTexture()
{
    skyTexture.clear();
}

void Scene::update(float dt)
{
    rootNode->update(dt);

    //cameras may not necessarily be a part of the scene heirarchy, so their matrices are updated here
    camera->update(dt);
    camera->updateCameraMatrices();
}

void Scene::render()
{

}

void Scene::addNode(SceneNodePtr node)
{
    if(node->sceneNodeType == SceneNodeType::Light)
    {
        auto light = node.staticCast<jah3d::LightNode>();
        lights.append(light);
    }
}

void Scene::removeNode(SceneNodePtr node)
{
    if(node->sceneNodeType == SceneNodeType::Light)
    {
        lights.removeOne(node.staticCast<jah3d::LightNode>());
    }
}

void Scene::setCamera(CameraNodePtr cameraNode)
{
    camera = cameraNode;
}

ScenePtr Scene::create()
{
    ScenePtr scene(new Scene());
    scene->rootNode->setScene(scene);

    return scene;
}

}
