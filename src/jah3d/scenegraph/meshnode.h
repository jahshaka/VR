#ifndef MESHNODE_H
#define MESHNODE_H

#include <QSharedPointer>
#include "../core/scenenode.h"
//#include "../graphics/mesh.h"
//#include "../graphics/material.h"

namespace jah3d
{

class Mesh;
class Material;

class MeshNode:public SceneNode
{
public:
    Mesh* mesh;
    QSharedPointer<Material> material;

    static QSharedPointer<jah3d::MeshNode> create()
    {
        return QSharedPointer<jah3d::MeshNode>(new jah3d::MeshNode());
    }

    void setMesh(QString source);
    void setMesh(Mesh* mesh);
    void setMaterial(QSharedPointer<jah3d::Material> material);
    QSharedPointer<jah3d::Material> getMaterial()
    {
        return material;
    }

private:
    MeshNode()
    {
        mesh = nullptr;
        sceneNodeType = SceneNodeType::Mesh;
    }
};

typedef QSharedPointer<jah3d::MeshNode> MeshNodePtr;


}

#endif // MESHNODE_H
