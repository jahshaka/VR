#include "playervrcontroller.h"
#include "../irisgl/src/graphics/graphicsdevice.h"
#include "../irisgl/src/graphics/renderitem.h"
#include "../irisgl/src/graphics/material.h"
#include "../irisgl/src/graphics/mesh.h"
#include "../irisgl/src/graphics/model.h"
#include "../irisgl/src/graphics/shader.h"
#include "../irisgl/src/materials/defaultmaterial.h"
#include "../irisgl/src/vr/vrdevice.h"
#include "../irisgl/src/vr/vrmanager.h"
#include "../irisgl/src/scenegraph/scene.h"
#include "../irisgl/src/scenegraph/scenenode.h"
#include "../irisgl/src/scenegraph/meshnode.h"
#include "../irisgl/src/scenegraph/grabnode.h"
#include "../irisgl/src/core/irisutils.h"
#include "../irisgl/src/math/mathhelper.h"
#include "../irisgl/src/scenegraph/cameranode.h"
#include "../core/keyboardstate.h"
#include "../irisgl/src/graphics/renderlist.h"
#include "../irisgl/src/content/contentmanager.h"
#include "../commands/transfrormscenenodecommand.h"
#include "../uimanager.h"
#include "irisgl/extras/Materials.h"
#include <QtMath>
#include "hand.h"

void RightHand::update(float dt)
{
	auto turnSpeed = 4.0f;
	hoverDist = 1000;
	// RIGHT CONTROLLER
	auto rightTouch = iris::VrManager::getDefaultDevice()->getTouchController(1);
	if (rightTouch->isTracking()) {

		auto device = iris::VrManager::getDefaultDevice();

		// rotate camera
		auto dir = rightTouch->GetThumbstick();
		auto yaw = camera->getLocalRot().toEulerAngles().y();
		yaw += -dir.x() * turnSpeed;
		auto yawRot = QQuaternion::fromEulerAngles(0, yaw, 0);
		camera->setLocalRot(yawRot);

		QMatrix4x4 world;
		world.setToIdentity();
		world.translate(device->getHandPosition(1));
		world.rotate(device->getHandRotation(1));
		//camera->setLocalScale(QVector3D(2,2,2));
		rightHandMatrix = camera->getGlobalTransform() * world;

		{
			// Handle picking and movement of picked objects
			iris::PickingResult pick;
			QMatrix4x4 beamOffset;
			beamOffset.setToIdentity();
			beamOffset.translate(0.0, -0.05f, 0.0);
			if (controller->rayCastToScene(rightHandMatrix * beamOffset, pick)) {
				auto dist = qSqrt(pick.distanceFromStartSqrd);
				hoverDist = dist;

				//qDebug() << "hit at dist: " << dist;
				//rightBeamRenderItem->worldMatrix.scale(1, 1, dist * (1.0f / 0.55f ));// todo: remove magic 0.55
				//rightBeamRenderItem->worldMatrix.scale(1, 1, dist);// todo: remove magic 0.55
				hoveredNode = controller->getObjectRoot(pick.hitNode);
				// Pick a node if the trigger is down
				if (rightTouch->getIndexTrigger() > 0.1f && !grabbedNode)
				{
					grabbedNode = hoveredNode;
					rightPos = grabbedNode->getLocalPos();
					rightRot = grabbedNode->getLocalRot();
					rightScale = grabbedNode->getLocalScale();

					//calculate offset
					rightNodeOffset = rightHandMatrix.inverted() * grabbedNode->getGlobalTransform();

					auto handleNode = controller->findGrabNode(grabbedNode);
					if (!!handleNode) {
						//rightNodeOffset = grabbedNode->getGlobalTransform().inverted() * handleNode->getGlobalTransform();
						rightNodeOffset = handleNode->getGlobalTransform().inverted() * grabbedNode->getGlobalTransform();
					}

					// should accept hit point
					{

						if (grabbedNode->isPhysicsBody && UiManager::isSimulationRunning) {
							scene->getPhysicsEnvironment()->createPickingConstraint(
								grabbedNode->getGUID(),
								iris::PhysicsHelper::btVector3FromQVector3D(grabbedNode->getGlobalPosition()),
								QVector3D(),
								QVector3D());
						}
					}
				}

			}
			else
			{
				//rightBeamRenderItem->worldMatrix.scale(1, 1, 100.f * (1.0f / 0.55f));
				hoveredNode.clear();
			}

			// trigger released
			if (rightTouch->getIndexTrigger() < 0.1f && !!grabbedNode)
			{
				// release node
				grabbedNode.clear();
				rightNodeOffset.setToIdentity(); // why bother?

				scene->getPhysicsEnvironment()->cleanupPickingConstraint();
			}

			// update picked node
			if (!!grabbedNode) {

				// calculate the global position
				auto nodeGlobal = rightHandMatrix * rightNodeOffset;

				// LAZLO's CODE
				if (grabbedNode->isPhysicsBody) {
					scene->getPhysicsEnvironment()->updatePickingConstraint(nodeGlobal);
				}
				else {
					

					// calculate position relative to parent
					auto localTransform = grabbedNode->parent->getGlobalTransform().inverted() * nodeGlobal;

					QVector3D pos, scale;
					QQuaternion rot;
					// decompose matrix to assign pos, rot and scale
					iris::MathHelper::decomposeMatrix(localTransform,
						pos,
						rot,
						scale);

					grabbedNode->setLocalPos(pos);
					rot.normalize();
					grabbedNode->setLocalRot(rot);
					grabbedNode->setLocalScale(scale);

				}
			}

			//scene->geometryRenderList->add(rightBeamRenderItem);
			//scene->geometryRenderList->add(rightHandRenderItem);
			//scene->geometryRenderList->submitMesh(handMesh, handMaterial, rightHandMatrix);
		}

		submitItemsToScene();
	}
}

void RightHand::loadAssets(iris::ContentManagerPtr content)
{
	handModel = content->loadModel(IrisUtils::getAbsoluteAssetPath("app/models/right_hand_anims.fbx"));
	auto mat = iris::DefaultMaterial::create();
	mat->setDiffuseColor(Qt::white);
	mat->enableFlag("SKINNING_ENABLED");
	handMaterial = mat;


	beamMesh = content->loadMesh(
		IrisUtils::getAbsoluteAssetPath("app/content/models/beam.obj"));
	sphereMesh = content->loadMesh(
		IrisUtils::getAbsoluteAssetPath("app/content/primitives/sphere.obj"));
	auto colorMat = iris::ColorMaterial::create();
	colorMat->setColor(QColor(255, 100, 100));
	beamMaterial = colorMat;
}

void RightHand::submitItemsToScene()
{
	QMatrix4x4 scale;
	scale.setToIdentity();
	
	scale.rotate(180, 0.0, 1.0, 0.0);
	scale.rotate(90, 0.0, 0.0, 1.0);
	scale.scale(0.1f, 0.1f, 0.1f);
	scene->geometryRenderList->submitModel(handModel, handMaterial, rightHandMatrix * scale);

	// beam
	if (!grabbedNode) {
		QMatrix4x4 beamMatrix;
		beamMatrix.setToIdentity();
		beamMatrix.translate(0.0, -0.05f, 0.0);
		beamMatrix.scale(0.4f, 0.4f, hoverDist);
		scene->geometryRenderList->submitMesh(beamMesh, beamMaterial, rightHandMatrix * beamMatrix);

		QMatrix4x4 sphereMatrix;
		sphereMatrix.setToIdentity();
		sphereMatrix.translate(0.0, -0.05f, 0.0);
		sphereMatrix.scale(0.02f);
		scene->geometryRenderList->submitMesh(sphereMesh, beamMaterial, rightHandMatrix * sphereMatrix);


		QMatrix4x4 tipMatrix;
		tipMatrix.setToIdentity();
		tipMatrix.translate(0.0, -0.05f, -hoverDist);
		tipMatrix.scale(0.02f);
		scene->geometryRenderList->submitMesh(sphereMesh, beamMaterial, rightHandMatrix * tipMatrix);

	}
	else {

	}
}

void Hand::init(iris::ScenePtr scene, iris::CameraNodePtr cam, iris::ViewerNodePtr viewer)
{
	this->scene = scene;
	this->camera = cam;
	this->viewer = viewer;
}

void Hand::setCamera(iris::CameraNodePtr camera)
{
	this->camera = camera;
}
