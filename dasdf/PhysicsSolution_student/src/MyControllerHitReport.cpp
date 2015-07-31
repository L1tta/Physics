#include "MyControllerHitReport.h"

void MyControllerHitReport::onShapeHit(const PxControllerShapeHit &hit)
{
	//gets a reference to a structure which tells us what has been hit and where
	//get the acter from the shape we hit
	PxRigidActor* actor = hit.shape->getActor();
	//get the normal of the thing we hit and store it so the player controller can respond correctly
	_playerContactNormal = hit.worldNormal;
	//try to cast to a dynamic actor
	PxRigidDynamic* myActor = actor->is<PxRigidDynamic>();
	if (myActor)
	{
		//this is where we can apply forces to things we hit
	}
}

//other collision functions which we must overload //these handle collision	with other controllers and hitting obstacles
void MyControllerHitReport::onControllerHit(const PxControllersHit &hit){};
//Called when current controller hits another controller. More...
void MyControllerHitReport::onObstacleHit(const PxControllerObstacleHit &hit){};
PxVec3 MyControllerHitReport::getPlayerContactNormal(){ return _playerContactNormal; };
void MyControllerHitReport::clearPlayerContactNormal(){ _playerContactNormal = PxVec3(0, 0, 0); };