#ifndef PHYSICS_H_
#define PHYSICS_H_
#define GLM_SWIZZLE

#include "Application.h"
#include "Camera.h"

#include "Callback.h"
#include "MyControllerHitReport.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>

#include "Ragdoll.h"
#include <vector>
#include <iostream>

#include "ParticleEmitter.h"
#include "ParticleFluidEmitter.h"

using namespace physx;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

class MyControllerHitReport;
class Callback;

class Physics : public Application
{
public:
    virtual bool startup();
    virtual void shutdown();
    virtual bool update();
    virtual void draw();

	void upDatePhysx(float a_deltaTime);
    void setupPhysx();
    void setupTutorial1();
	void GizmosUpdate();

	bool fall;
	float timer;

    FlyCamera m_camera;

	PxFoundation* g_PhysicsFoundation;
	PxPhysics* g_Physics;
	PxScene* g_PhysicsScene;
	PxDefaultErrorCallback gDefaultErrorCallback;
	PxDefaultAllocator gDefaultAllocatorCallback;
	PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;
	PxMaterial* g_PhysicsMaterial;
	PxMaterial* g_boxMaterial;
	PxCooking* g_PhysicsCooker;
	PxRigidStatic* StaticBox;
	PxRigidStatic* StaticBox2;
	PxRigidDynamic* dynamicActor;
	PxRigidDynamic* dynamicActor2;
	PxArticulation* ragDollArticulation;
	PxRigidStatic* cbox1;
	PxRigidStatic* cbox2;
	PxRigidStatic* cbox3;
	PxRigidStatic* cbox4;
	ParticleEmitter* m_particleEmitter;
	ParticleFluidEmitter* m_particleEmitter2;
	PxRigidStatic* TriggerBox;
	PxControllerManager* gCharacterManager;
	MyControllerHitReport* myHitReport;
	PxController* gPlayerController;
	Callback* Eventc;

	float _characterYVelocity;
	float _characterRotation;
	int _playerGravity;
	vec4 DColour;
};

class myAllocator : public PxAllocatorCallback
{
public:
	virtual ~myAllocator() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* pointer = _aligned_malloc(size, 16);
		return pointer;
	}
	virtual void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}
};

#endif //PHYSICS_H_