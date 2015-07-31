#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>
#include "glm/glm.hpp"
using namespace physx;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

class Callback : public PxSimulationEventCallback
{
public:
	Callback();
	virtual ~Callback();

	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)override;
	virtual void onWake(PxActor** actors, PxU32 count)override;
	virtual void onSleep(PxActor** actors, PxU32 count)override;
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)override;

	PxRigidDynamic* getActor1;
	PxRigidDynamic* getActor3;
	PxRigidStatic* getActor2;
	vec4 Colour1;
	vec4 Colour2;
	bool trigger;
	bool trigger2;
};

#endif //_CALLBACK_H_