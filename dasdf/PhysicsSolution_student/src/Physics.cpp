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

	DColour = vec4(1, 1, 1, 1);
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
	float density4 = 10;
	PxBoxGeometry box4(5, 1, 5);
	PxTransform TBox(PxVec3(0, 8, 0));
	TriggerBox = PxCreateStatic(*g_Physics, TBox, box4, *g_PhysicsMaterial);
	PxShape* Shape;
	TriggerBox->getShapes(&Shape, 1);
	Shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	Shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	g_PhysicsScene->addActor(*TriggerBox);

	//fluid 
	PxTransform pose1 = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane1 = PxCreateStatic(*g_Physics, pose1, PxPlaneGeometry(), *g_PhysicsMaterial);
	const PxU32 numShapes = plane1->getNbShapes();
	g_PhysicsScene->addActor(*plane1);

	PxBoxGeometry side1(4.5, 1, .5);
	PxBoxGeometry side2(.5, 1, 4.5);

	pose1 = PxTransform(PxVec3(0.0f, 0.5, 4.0f));
	cbox1 = PxCreateStatic(*g_Physics, pose1, side1, *g_PhysicsMaterial);
	g_PhysicsScene->addActor(*cbox1);
	PxVec3 c1 = pose1.p;
	vec3 c1p;
	c1p.x = c1.x;
	c1p.y = c1.y;
	c1p.z = c1.z;
	Gizmos::addAABBFilled(c1p, vec3(1, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c1p, vec3(2, 1, 1), vec4(0, 0, 0, 1));

	pose1 = PxTransform(PxVec3(0.0f, 0.5, -4.0f));
	cbox2 = PxCreateStatic(*g_Physics, pose1, side1, *g_PhysicsMaterial);
	g_PhysicsScene->addActor(*cbox2);
	PxVec3 c2 = pose1.p;
	vec3 c2p;
	c2p.x = c2.x;
	c2p.y = c2.y;
	c2p.z = c2.z;
	Gizmos::addAABBFilled(c2p, vec3(1, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c2p, vec3(2, 1, 1), vec4(0, 0, 0, 1));

	pose1 = PxTransform(PxVec3(4.0f, 0.5, 0));
	cbox3 = PxCreateStatic(*g_Physics, pose1, side2, *g_PhysicsMaterial);
	g_PhysicsScene->addActor(*cbox3);
	PxVec3 c3 = pose1.p;
	vec3 c3p;
	c3p.x = c3.x;
	c3p.y = c3.y;
	c3p.z = c3.z;
	Gizmos::addAABBFilled(c3p, vec3(1, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c3p, vec3(2, 1, 1), vec4(0, 0, 0, 1));

	pose1 = PxTransform(PxVec3(-4.0f, 0.5, 0));
	cbox4 = PxCreateStatic(*g_Physics, pose1, side2, *g_PhysicsMaterial);
	g_PhysicsScene->addActor(*cbox4);
	PxVec3 c4 = pose1.p;
	vec3 c4p;
	c4p.x = c4.x;
	c4p.y = c4.y;
	c4p.z = c4.z;
	Gizmos::addAABBFilled(c4p, vec3(1, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c4p, vec3(2, 1, 1), vec4(0, 0, 0, 1));
	//fluid
	//PxParticleSystem* pf;
	//// create particle system in PhysX SDK
	//// set immutable properties.
	//PxU32 maxParticles = 4000;
	//bool perParticleRestOffset = false;
	//pf = g_Physics->createParticleSystem(maxParticles, perParticleRestOffset);
	//pf->setDamping(0.1);
	//pf->setParticleMass(.1);
	//pf->setRestitution(0);
	//pf->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	//if (pf)
	//{
	//	g_PhysicsScene->addActor(*pf);
	//	m_particleEmitter = new ParticleEmitter(maxParticles,
	//		PxVec3(0, 10, 0), pf, .01);
	//	m_particleEmitter->setStartVelocityRange(-2.0f, 0, -2.0f,
	//		2.0f, 0.0f, 2.0f);
	//}

	//create our particle system
	PxParticleFluid* pf;
	// create particle system in PhysX SDK
	// set immutable properties.
	PxU32 maxParticles = 4000;
	bool perParticleRestOffset = false;
	pf = g_Physics->createParticleFluid(maxParticles, perParticleRestOffset);
	pf->setRestParticleDistance(.3f);
	pf->setDynamicFriction(0.1);
	pf->setStaticFriction(0.1);
	pf->setDamping(0.1);
	pf->setParticleMass(.1);
	pf->setRestitution(0);
	//pf->setParticleReadDataFlag(PxParticleReadDataFlag::eDENSITY_BUFFER,
	// true);
	pf->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	pf->setStiffness(100);
	if (pf)
	{
		g_PhysicsScene->addActor(*pf);
		m_particleEmitter2 = new ParticleFluidEmitter(maxParticles,
			PxVec3(0, 10, 0), pf, .1);
		m_particleEmitter2->setStartVelocityRange(-0.001f, -250.0f, -0.001f,
			0.001f, -250.0f, 0.001f);
	}

	//add player capsule
	myHitReport = new MyControllerHitReport();
	gCharacterManager = PxCreateControllerManager(*g_PhysicsScene);
	PxCreateControllerManager(*g_PhysicsScene);
	//describe our controller...
	PxCapsuleControllerDesc desc;
	desc.height = 1.6f;
	desc.radius = 0.4f;
	desc.position.set(0, 0, 0);
	desc.material = g_Physics->createMaterial(0.5f, 0.5f, .5f);
	desc.reportCallback = myHitReport; //connect it to our collision detection routine
	desc.density = 10;
	//create the layer controller
	gPlayerController = gCharacterManager->createController(desc);
	gPlayerController->setPosition(PxExtendedVec3(5,5,5));
	//set up some variables to control our player with
	_characterYVelocity = 0; //initialize character velocity
	_characterRotation = 0; //and rotation
	_playerGravity = -0.5f; //set up the player gravity
	myHitReport->clearPlayerContactNormal(); //initialize the contact normal (what we are in contact with)
}


void Physics::setupPhysx()
{
	Eventc = new Callback();
	PxAllocatorCallback *myCallback = new myAllocator();
	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, gDefaultErrorCallback);
	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxTolerancesScale());
	PxInitExtensions(*g_Physics);
	//create physics material 
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f,.5f);
	PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.simulationEventCallback = Eventc;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_PhysicsScene = g_Physics->createScene(sceneDesc);
	g_PhysicsScene->setFlag(PxSceneFlag::eENABLE_KINEMATIC_PAIRS, true);
	g_PhysicsScene->setFlag(PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS, true);
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

	Eventc->getActor1 = dynamicActor;
	Eventc->getActor2 = TriggerBox;
	Eventc->getActor3 = dynamicActor2;

    Gizmos::clear();

    float dt = (float)glfwGetTime();
    glfwSetTime(0.0);
	upDatePhysx(dt);
	timer += dt;

	if (timer >= 4)
	{
		Eventc->Colour1 = vec4(1, 1, 1, 1);
		Eventc->trigger = false;
		Eventc->Colour2 = vec4(1, 1, 1, 1);
		Eventc->trigger2 = false;
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

	if (m_particleEmitter2)
	{
		m_particleEmitter2->update(dt);
		//render all our particles
		m_particleEmitter2->renderParticles();
	}

	bool onGround; //set to true if we are on the ground
	float movementSpeed = 10.0f; //forward and back movement speed
	float rotationSpeed = 1.0f; //turn speed
	//check if we have a contact normal. if y is greater than .3 we assume this is solid ground.This is a rather primitive way to do this.Can you do better ?
	if (myHitReport->getPlayerContactNormal().y > 0.3f)
	{
		_characterYVelocity = -0.1f;
		onGround = true;
	}
	else
	{
		_characterYVelocity += _playerGravity * dt;
		onGround = false;
	}
	myHitReport->clearPlayerContactNormal();
	const PxVec3 up(0, 1, 0);
	//scan the keys and set up our intended velocity based on a global transform
	PxVec3 velocity(0, _characterYVelocity, 0);
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_UP) == GLFW_PRESS)
	{
		velocity.x -= movementSpeed*dt;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		velocity.x += movementSpeed*dt;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		_characterRotation += rotationSpeed * dt;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		_characterRotation -= rotationSpeed * dt;
	}
	////To do.. add code to control z movement and jumping
	float minDistance = 0.001f;
	PxControllerFilters filter;
	////make controls relative to player facing
	PxQuat rotation(_characterRotation, PxVec3(0, 1, 0));
	////move the controller
	gPlayerController->move(rotation.rotate(velocity), minDistance, dt, filter);

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
	Gizmos::addAABBFilled(Gpos, vec3(1, 1, 1), Eventc->Colour1, &result);
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
	Gizmos::addAABBFilled(Gpos2, vec3(1, 1, 1), Eventc->Colour2, &result2);
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
	//container
	PxTransform transform4 = cbox1->getGlobalPose();
	PxVec3 c1 = transform4.p;
	vec3 c1p;
	c1p.x = c1.x;
	c1p.y = c1.y;
	c1p.z = c1.z;
	Gizmos::addAABBFilled(c1p, vec3(5, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c1p, vec3(5.001, 1.001, 1.001), vec4(0, 0, 0, 1));

	PxTransform transform5 = cbox2->getGlobalPose();
	PxVec3 c2 = transform5.p;
	vec3 c2p;
	c2p.x = c2.x;
	c2p.y = c2.y;
	c2p.z = c2.z;
	Gizmos::addAABBFilled(c2p, vec3(5, 1, 1), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c2p, vec3(5.001, 1.001, 1.001), vec4(0, 0, 0, 1));

	PxTransform transform6 = cbox3->getGlobalPose();
	PxVec3 c3 = transform6.p;
	vec3 c3p;
	c3p.x = c3.x;
	c3p.y = c3.y;
	c3p.z = c3.z;
	Gizmos::addAABBFilled(c3p, vec3(1, 1, 5), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c3p, vec3(1.001, 1.001, 5.001), vec4(0, 0, 0, 1));

	PxTransform transform7 = cbox4->getGlobalPose();
	PxVec3 c4 = transform7.p;
	vec3 c4p;
	c4p.x = c4.x;
	c4p.y = c4.y;
	c4p.z = c4.z;
	Gizmos::addAABBFilled(c4p, vec3(1, 1, 5), vec4(1, 1, 1, 1));
	Gizmos::addAABB(c4p, vec3(1.001, 1.001, 5.001), vec4(0, 0, 0, 1));

	//trigger volume
	PxTransform transform8 = TriggerBox->getGlobalPose();
	PxVec3 c5 = transform8.p;
	vec3 c5p;
	c5p.x = c5.x;
	c5p.y = c5.y;
	c5p.z = c5.z;
	Gizmos::addAABB(c5p, vec3(5.001, 1.001, 5.001), vec4(0, 0, 0, 1));

	//Capsule
	const PxExtendedVec3 c6 = gPlayerController->getPosition();
	PxTransform transform9 = gPlayerController->getActor()->getGlobalPose();
	vec3 c7p;
	vec3 c6p;
	c6p.x = c6.x;
	c6p.y = c6.y;
	c6p.z = c6.z;
	glm::quat rot1;
	const float a = _characterRotation;
	PxVec3 up = gPlayerController->getUpDirection();
	vec3 upvec;
	upvec.x = up.x;
	upvec.y = up.y;
	upvec.z = up.z;
	rot1 = glm::angleAxis(a,upvec);
	const mat4 resultp = glm::translate(c6p) * glm::toMat4(rot1);
	Gizmos::addAABBFilled(c6p, vec3(0.501, 1.001, 0.501), vec4(0, 0, 0, 1), &resultp);

}