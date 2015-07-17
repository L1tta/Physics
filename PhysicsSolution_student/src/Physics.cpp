#include "Physics.h"

#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include "Gizmos.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <stdio.h>     
#include <stdlib.h>    
#include <time.h>      

#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))


bool Physics::startup()
{
    if (Application::startup() == false)
    {
        return false;
    }

	srand(time(NULL));

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    Gizmos::create();

	timer = 0;
	fall = false;
    m_camera = FlyCamera(1280.0f / 720.0f, 10.0f);
    m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
    m_camera.sensitivity = 3;

    setupPhysx();
    setupTutorial1();

    return true;
}

void Physics::setupTutorial1()
{
	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi*1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_Physics, pose, PxPlaneGeometry(), *g_PhysicsMaterial);
	//add it to the physX scene
	g_PhysicsScene->addActor(*plane);
	//create gizmos
	Gizmos::addAABB(vec3(0, 0, 0), vec3(20, 0, 20), vec4(0, 0, 0, 1));
	//add a box
	float density = 10;
	PxBoxGeometry box(1, 1, 1);
	PxTransform transform(PxVec3(0, 5, 0));
	dynamicActor = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density);
	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor);
	//create gizmos
	PxVec3 pos = transform.p;
	vec3 Gpos;
	Gpos.x = pos.x;
	Gpos.y = pos.y;
	Gpos.z = pos.z;
	Gizmos::addAABBFilled(Gpos, vec3(0, 5, 0), vec4(1, 1, 1, 1));
	Gizmos::addAABB(Gpos, vec3(0, 5, 0), vec4(0, 0, 0, 1));
	//add a box
	float density2 = 10;
	PxBoxGeometry box2(1, 1, 1);
	PxTransform transform2(PxVec3(1, 10, 0));
	dynamicActor2 = PxCreateDynamic(*g_Physics, transform2, box2, *g_PhysicsMaterial, density2);
	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor2);
	//create gizmos
	PxVec3 pos2 = transform2.p;
	vec3 Gpos2;
	Gpos2.x = pos2.x;
	Gpos2.y = pos2.y;
	Gpos2.z = pos2.z;
	Gizmos::addAABBFilled(Gpos2, vec3(0, 5, 0), vec4(1, 1, 1, 1));
	Gizmos::addAABB(Gpos2, vec3(0, 5, 0), vec4(0, 0, 0, 1));
	//add ragdoll
	ragDollArticulation = Ragdoll::makeRagdoll(g_Physics, ragdollData, PxTransform(PxVec3(0, 10, 0)), 0.1f, g_PhysicsMaterial);
	g_PhysicsScene->addArticulation(*ragDollArticulation);
	//create gizmos
	PxArticulationLink* Links[64];
	int linkcount = ragDollArticulation->getLinks(Links, 64);
	for (int i = 0; i < linkcount; i++)
	{
		physx::PxShape* shape;
		Links[i]->getShapes(&shape, 1);
		physx::PxCapsuleGeometry capGeo;
		shape->getCapsuleGeometry(capGeo);
		PxTransform RagTrans = Links[i]->getGlobalPose();
		PxVec3 ragPos = RagTrans.p;
		vec3 GragPos;
		GragPos.x = ragPos.x;
		GragPos.y = ragPos.y;
		GragPos.z = ragPos.z;
		glm::quat ragRot;
		ragRot.x = RagTrans.q.x;
		ragRot.y = RagTrans.q.y;
		ragRot.z = RagTrans.q.z;
		ragRot.w = RagTrans.q.w;
		const mat4 ragResult = glm::translate(Gpos) * glm::toMat4(ragRot);
		Gizmos::addAABBFilled(Gpos, vec3(1, 1, 1), vec4(1, 1, 1, 1), &ragResult);
		Gizmos::addAABB(Gpos, vec3(1.001, 1.001, 1.001), vec4(0, 0, 0, 1), &ragResult);
	}
	//add a static box
	float density3 = 10;
	PxBoxGeometry box3(2, 1, 1);
	PxTransform SBox(PxVec3(0, 3, 0));
	StaticBox = PxCreateStatic(*g_Physics, SBox, box3, *g_PhysicsMaterial);
	//add it to the physX scene
	g_PhysicsScene->addActor(*StaticBox);
	//create gizmos
	PxVec3 Spos = SBox.p;
	vec3 SGpos;
	SGpos.x = Spos.x;
	SGpos.y = Spos.y;
	SGpos.z = Spos.z;
	Gizmos::addAABBFilled(SGpos, vec3(2, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(SGpos, vec3(2, 1, 1), vec4(0, 0, 0, 1));
	//add Trigger Volume
}


void Physics::setupPhysx()
{
	PxAllocatorCallback *myCallback = new myAllocator();
	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, gDefaultErrorCallback);
	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxTolerancesScale());
	PxInitExtensions(*g_Physics);
	//create physics material 
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f,.5f);
	PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_PhysicsScene = g_Physics->createScene(sceneDesc);
}

void Physics::shutdown()
{
	g_PhysicsScene->release();
	g_Physics->release();
	g_PhysicsFoundation->release();
    Gizmos::destroy();
    Application::shutdown();
}

bool Physics::update()
{
    if (Application::update() == false)
    {
        return false;
    }

	if (fall == false)
	{
		fall = true;
	}

    Gizmos::clear();

    float dt = (float)glfwGetTime();
    glfwSetTime(0.0);
	upDatePhysx(dt);
	timer += dt;

	if (timer >= 4)
	{
		fall = false;
	}

    vec4 white(1);
    vec4 black(0, 0, 0, 1);

    for (int i = 0; i <= 20; ++i)
    {
        Gizmos::addLine(vec3(-10 + i, -0.01, -10), vec3(-10 + i, -0.01, 10),
            i == 10 ? white : black);
        Gizmos::addLine(vec3(-10, -0.01, -10 + i), vec3(10, -0.01, -10 + i),
            i == 10 ? white : black);
    }

    m_camera.update(dt);

    return true;
}

void Physics::upDatePhysx(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}
	g_PhysicsScene->simulate(a_deltaTime);
	while (g_PhysicsScene->fetchResults() == false)
	{
		// don’t need to do anything here yet but we have to fetch results
	}
}

void Physics::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GizmosUpdate();
    Gizmos::draw(m_camera.proj, m_camera.view);
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Physics::GizmosUpdate()
{
	if (fall == false)
	{
		PxArticulationLink* Links[64];
		int linkcount = ragDollArticulation->getLinks(Links, 64);
		for (int i = 0; i < linkcount; i++)
		{
			Links[i]->setGlobalPose(PxTransform(PxVec3((rand() % 2), (rand() % 17 + 15), (rand() % 2))));
		}
		PxTransform reset1(PxVec3((rand() % 2), (rand() % 10 + 5), (rand() % 2)));
		PxTransform reset2(PxVec3((rand() % 2), (rand() % 10 + 5), (rand() % 2)));
		dynamicActor->setGlobalPose(reset1);
		dynamicActor2->setGlobalPose(reset2);
		timer = 0;
	}

	//plane
	Gizmos::addAABBFilled(vec3(0, 0, 0), vec3(20, 0, 20), vec4(0, 0, 0, 1));
	//box
	PxTransform transform = dynamicActor->getGlobalPose();
	PxVec3 pos = transform.p;
	vec3 Gpos;
	Gpos.x = pos.x;
	Gpos.y = pos.y;
	Gpos.z = pos.z;
	glm::quat rot;
	rot.x = transform.q.x;
	rot.y = transform.q.y;
	rot.z = transform.q.z;
	rot.w = transform.q.w;
	const mat4 result = glm::translate(Gpos) * glm::toMat4(rot);
	Gizmos::addAABBFilled(Gpos, vec3(1, 1, 1), vec4(1, 1, 1, 1), &result);
	Gizmos::addAABB(Gpos, vec3(1.001, 1.001, 1.001), vec4(0, 0, 0, 1), &result);
	//box2
	PxTransform transform2 = dynamicActor2->getGlobalPose();
	PxVec3 pos2 = transform2.p;
	vec3 Gpos2;
	Gpos2.x = pos2.x;
	Gpos2.y = pos2.y;
	Gpos2.z = pos2.z;
	glm::quat rot2;
	rot2.x = transform2.q.x;
	rot2.y = transform2.q.y;
	rot2.z = transform2.q.z;
	rot2.w = transform2.q.w;
	const mat4 result2 = glm::translate(Gpos2) * glm::toMat4(rot2);
	Gizmos::addAABBFilled(Gpos2, vec3(1, 1, 1), vec4(1, 1, 1, 1), &result2);
	Gizmos::addAABB(Gpos2, vec3(1.001, 1.001, 1.001), vec4(0, 0, 0, 1), &result2);
	//Ragdoll
	PxArticulationLink* Links[64];
	int linkcount = ragDollArticulation->getLinks(Links, 64);
	for (int i = 0; i < linkcount; i++)
	{
		physx::PxShape* shape;
		Links[i]->getShapes(&shape, 1);
		physx::PxCapsuleGeometry capGeo;
		shape->getCapsuleGeometry(capGeo);
		PxTransform RagTrans = Links[i]->getGlobalPose();
		PxVec3 ragPos = RagTrans.p;
		vec3 GragPos;
		GragPos.x = ragPos.x;
		GragPos.y = ragPos.y;
		GragPos.z = ragPos.z;
		glm::quat ragRot;
		ragRot.x = RagTrans.q.x;
		ragRot.y = RagTrans.q.y;
		ragRot.z = RagTrans.q.z;
		ragRot.w = RagTrans.q.w;
		const mat4 ragResult = glm::translate(GragPos) * glm::toMat4(ragRot);
		Gizmos::addSphere(GragPos, 0.35, 10, 10, vec4(0, 0, 1, 1), &ragResult);
	}
	//static box
	PxTransform transform3 = StaticBox->getGlobalPose();
	PxVec3 Spos = transform3.p;
	vec3 SGpos;
	SGpos.x = Spos.x;
	SGpos.y = Spos.y;
	SGpos.z = Spos.z;
	Gizmos::addAABBFilled(SGpos, vec3(2, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(SGpos, vec3(2.001, 1.001, 1.001), vec4(0, 0, 0, 1));
}

//PxArticulation* Physics::makeRagdoll(PxPhysics* g_Physics, RagdollNode** nodeArray, PxTransform worldPos, float scaleFactor, PxMaterial* ragdollMaterial)
//{
//	//create the articulation for our ragdoll
//	PxArticulation *articulation = g_Physics->createArticulation();
//	RagdollNode** currentNode = nodeArray;
//	//while there are more nodes to process...
//	while (*currentNode != NULL)
//	{
//		//get a pointer to the current node
//		currentNode++;
//		//get a pointer to the current node
//		RagdollNode* currentNodePtr = *currentNode;
//		//create a pointer ready to hold the parent node pointer if there is one
//		RagdollNode* parentNode = nullptr;
//		//get scaled values for capsule
//		float radius = currentNodePtr->radius*scaleFactor;
//		float halfLength = currentNodePtr->halfLength*scaleFactor;
//		float childHalfLength = radius + halfLength;
//		float parentHalfLength = 0; //will be set later if there is a parent
//		//get a pointer to the parent
//		PxArticulationLink* parentLinkPtr = NULL;
//		currentNodePtr->scaledGobalPos = worldPos.p;
//
//		if (currentNodePtr->parentNodeIdx != -1)
//		{
//			//if there is a parent then we need to work out our local position for the link
//			//get a pointer to the parent node
//			parentNode = *(nodeArray + currentNodePtr->parentNodeIdx);
//			//get a pointer to the link for the parent
//			parentLinkPtr = parentNode->linkPtr;
//			parentHalfLength = (parentNode->radius + parentNode->halfLength) *scaleFactor;
//			//work out the local position of the node
//			PxVec3 currentRelative = currentNodePtr->childLinkPos * currentNodePtr->globalRotation.rotate(PxVec3(childHalfLength, 0, 0));
//			PxVec3 parentRelative = -currentNodePtr->parentLinkPos * parentNode->globalRotation.rotate(PxVec3(parentHalfLength, 0, 0));
//			currentNodePtr->scaledGobalPos = parentNode->scaledGobalPos - (parentRelative + currentRelative);
//		}
//
//		//build the transform for the link
//		PxTransform linkTransform = PxTransform(currentNodePtr->scaledGobalPos, currentNodePtr ->globalRotation);
//		//create the link in the articulation
//		PxArticulationLink* link = articulation->createLink(parentLinkPtr, linkTransform);
//		//add the pointer to this link into the ragdoll data so we have it for later when we want to link to it
//		currentNodePtr->linkPtr = link;
//		float jointSpace = .01f; //gap between joints
//		float capsuleHalfLength = (halfLength > jointSpace ? halfLength - jointSpace : 0) + .01f;
//		PxCapsuleGeometry capsule(radius, capsuleHalfLength);
//		link->createShape(capsule, *ragdollMaterial); //adds a capsule collider to the link
//		PxRigidBodyExt::updateMassAndInertia(*link, 50.0f); //adds some mass, mass should really be part of the data!
//
//		if (currentNodePtr->parentNodeIdx != -1)
//		{
//			//get the pointer to the joint from the link
//			joint = link->getInboundJoint();
//			//get the relative rotation of this link
//			PxQuat frameRotation = parentNode->globalRotation.getConjugate() * currentNodePtr->globalRotation;
//			//set the parent contraint frame
//			PxTransform parentConstraintFrame = PxTransform(PxVec3(currentNodePtr ->parentLinkPos * parentHalfLength, 0, 0), frameRotation);
//			//set the child constraint frame (this the constraint frame of the newly added link)
//			PxTransform thisConstraintFrame = PxTransform(PxVec3(currentNodePtr ->childLinkPos * childHalfLength, 0, 0));
//			//set up the poses for the joint so it is in the correct place
//			joint->setParentPose(parentConstraintFrame);
//			joint->setChildPose(thisConstraintFrame);
//			//set up some constraints to stop it flopping around
//			joint->setStiffness(20);
//			joint->setDamping(20);
//			joint->setSwingLimit(0.4f, 0.4f);
//			joint->setSwingLimitEnabled(true);
//			joint->setTwistLimit(-0.1f, 0.1f);
//			joint->setTwistLimitEnabled(true);
//		}
//	}
//	return articulation;
//}