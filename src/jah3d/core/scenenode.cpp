#include "scenenode.h"

namespace jah3d
{

SceneNode::SceneNode():
    pos(QVector3D(0,0,0)),
    scale(QVector3D(0,0,0)),
    rot(QQuaternion())

{
    sceneNodeType = SceneNodeType::Empty;
}

SceneNodePtr SceneNode::create()
{
    return QSharedPointer<SceneNode>(new SceneNode());
}

QString SceneNode::getName()
{
    return name;
}

SceneNodeType SceneNode::getSceneNodeType()
{
    return sceneNodeType;
}

void SceneNode::addChild(SceneNodePtr node)
{
    //todo: check if child is already a node
    auto self = sharedFromThis();

    children.append(node);
    node->setParent(self);
    node->setScene(self->scene);
    scene->addNode(node);
}

void SceneNode::removeFromParent()
{
    auto self = sharedFromThis();

    if(!parent.isNull())
        this->parent->removeChild(self);
}

void SceneNode::removeChild(SceneNodePtr node)
{
    children.removeOne(node);
    node->parent = QSharedPointer<SceneNode>(nullptr);
    node->scene = ScenePtr(nullptr);
    scene->removeNode(node);
}

void SceneNode::setParent(SceneNodePtr node)
{
    this->parent = node;
}

void SceneNode::setScene(ScenePtr scene)
{
    this->scene = scene;
}


}
