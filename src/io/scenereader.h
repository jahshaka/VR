#ifndef SCENEREADER_H
#define SCENEREADER_H

#include <QSharedPointer>
#include "assetiobase.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QJsonDocument>

#include "../irisgl/src/irisglfwd.h"
#include "../irisgl/src/core/scenenode.h"
#include "../irisgl/src/scenegraph/lightnode.h"

class SceneReader:public AssetIOBase
{
    QHash<QString,QList<iris::Mesh*>> meshes;
public:
    iris::ScenePtr readScene(QString filePath);

    iris::ScenePtr readScene(QJsonObject& projectObj);

    /**
     * Creates scene node from json data
     * @param nodeObj
     * @return
     */
    iris::SceneNodePtr readSceneNode(QJsonObject& nodeObj);


    void readAnimationData(QJsonObject& nodeObj,iris::SceneNodePtr sceneNode);

    /**
     * Reads pos, rot and scale properties from json object
     * if scale isnt available then it's set to (1,1,1) by default
     * @param nodeObj
     * @param sceneNode
     */
    void readSceneNodeTransform(QJsonObject& nodeObj,iris::SceneNodePtr sceneNode);

    /**
     * Creates mesh using scene node data
     * @param nodeObj
     * @return
     */
    iris::MeshNodePtr createMesh(QJsonObject& nodeObj);

    /**
     * Creates light from light node data
     * @param nodeObj
     * @return
     */
    iris::LightNodePtr createLight(QJsonObject& nodeObj);

    iris::LightType getLightTypeFromName(QString lightType);

    /**
     * Extracts material from node's json object.
     * Creates default material if one isnt defined in nodeObj
     * @param nodeObj
     * @return
     */
    iris::MaterialPtr readMaterial(QJsonObject& nodeObj);

    /**
     * Returns mesh from mesh file at index
     * if the mesh doesnt exist, nullptr is returned
     * @param filePath
     * @param index
     * @return
     */
    iris::Mesh* getMesh(QString filePath,int index);


};

#endif // SCENEREADER_H
