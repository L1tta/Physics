#include "GLFW/glfw3.h"
#include "gl_core_4_4.h"
#include "Gizmos.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <iostream>
#include "DIYPhysicsEngine.h"
#include "DIYmain.h"

DIYmain::DIYmain()
{

}
DIYmain::~DIYmain()
{

}
bool DIYmain::startup()
{
	if (glfwInit() == false)
	{
		return -1;
	}

	//	DIYPhysicsRocketSetup();
	//DIYPhysicsCollisionTutorial();
	//  SpringPhysicsTutorial();

	window = glfwCreateWindow(1080, 720, "Physics 2D", nullptr, nullptr);

	if (window == nullptr)
	{
		glfwTerminate();
		return -2;
	}

	glfwMakeContextCurrent(window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -3;
	}

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	Gizmos::create();
	prevTime = 0;

	m_camera = FlyCamera(1280.0f / 720.0f, 10.0f);
	m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
	m_camera.sensitivity = 3;

	//	DIYPhysicsRocketSetup();
	DIYPhysicsCollisionTutorial();
	//  SpringPhysicsTutorial();
	return true;
}
void DIYmain::shutdown()
{
	Gizmos::destroy();
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool DIYmain::update()
{
	// grab the time since the application started (in seconds)
	float time = (float)glfwGetTime();

	// calculate a delta time
	float deltaTime = time - prevTime;
	prevTime = time;

	Gizmos::clear();
	upDate2DPhysics(deltaTime);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i)
	{
		Gizmos::addLine(vec3(-10 + i, -0.01, -10), vec3(-10 + i, -0.01, 10),
			i == 10 ? white : black);
		Gizmos::addLine(vec3(-10, -0.01, -10 + i), vec3(10, -0.01, -10 + i),
			i == 10 ? white : black);
	}

	m_camera.update(deltaTime);

	return true;
}
void DIYmain::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.102f, 0.489f, 0.866f, 1);
	glEnable(GL_DEPTH_TEST);


	Gizmos::draw(m_camera.proj, m_camera.view);
	draw2DGizmo();
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void DIYmain::draw2DGizmo()
{
	Gizmos::draw2D(m_camera.view_proj);
}

glm::vec2 DIYmain::GetWorldMouse()
{
	int width = 0, height = 0;
	glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
	double mousex, mousey;
	glfwGetCursorPos(window, &mousex, &mousey);
	mousex /= width;
	mousey /= height;
	mousex -= 0.5;
	mousex *= 200.0f;
	mousey -= 0.5;
	mousey *= -1;
	mousey *= 200.0f * ((float)height / (float)width);
	glm::vec2 result = glm::vec2((float)mousex, (float)mousey);

	return result;
}

void DIYmain::upDate2DPhysics(float delta)
{
	BoxClass* box1 = (BoxClass*)physicsScene->actors[0];

	static glm::vec2 cm_to_anchor = glm::vec2();
	static bool grabbed = false;

	if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
	{
		if (!grabbed)
		{
			if (box1->isPointOver(GetWorldMouse())) // added this check
			{
				cm_to_anchor = GetWorldMouse() - box1->position;
				float sin_theta = sinf(-box1->rotation2D);
				float cos_theta = cosf(-box1->rotation2D);
				cm_to_anchor = glm::vec2(cos_theta * cm_to_anchor.x - sin_theta * cm_to_anchor.y,
					sin_theta * cm_to_anchor.x + cos_theta * cm_to_anchor.y);
				grabbed = true;
			}
		}
		else //added the else
		{
			//compute the anchor point
			float sin_theta = sinf(box1->rotation2D);
			float cos_theta = cosf(box1->rotation2D);
			glm::vec2 rot_local_pos = glm::vec2(cos_theta * cm_to_anchor.x - sin_theta * cm_to_anchor.y,
				sin_theta * cm_to_anchor.x + cos_theta * cm_to_anchor.y);

			glm::vec2 anchor = box1->position + rot_local_pos;

			//add force at anchor point towards the mouse
			box1->applyForceAtPoint(GetWorldMouse() - anchor, anchor);
			Gizmos::add2DLine(anchor, GetWorldMouse(), glm::vec4(0, 1, 1, 1));
		}
	}
	else
	{
		grabbed = false;
	}

	physicsScene->upDate();
	physicsScene->upDateGizmos();
	onUpdateRocket(delta);
}

void DIYmain::DIYPhysicsRocketSetup()
{
	//note - collision detection must be disabled in the physics engine for this to work.
	physicsScene = new DIYPhysicScene();
	physicsScene->collisionEnabled = false;
	physicsScene->gravity = glm::vec2(0, -.2);
	physicsScene->timeStep = .016f;
	rocket = new SphereClass(glm::vec2(-40, 0), glm::vec2(0, 0), 6.0f, 5, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(rocket);
}

void DIYmain::DIYPhysicsCollisionTutorial()
{
	//note - collision detection must be disabled in the physics engine for this to work.
	physicsScene = new DIYPhysicScene();
	physicsScene->collisionEnabled = true;
	physicsScene->timeStep = .016f;
	physicsScene->gravity = glm::vec2(0, -15);

	SphereClass* sphere1 = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(sphere1);
	sphere1->velocity = glm::vec2(10, 0);

	SphereClass* sphere2 = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(sphere2);
	sphere2->velocity = glm::vec2(15, 0);

	SphereClass* sphere3 = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(sphere3);

	SphereClass* sphere4 = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(sphere4);

	SphereClass* sphere5 = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	sphere5->is_static = true;
	physicsScene->addActor(sphere5);

	SphereClass* circle = new SphereClass(glm::vec2(0, 0), glm::vec2(0, 0), 1, 1, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(circle);

	/* BoxClass* box = new BoxClass(glm::vec2(-0, 20), glm::vec2(0, 0), 0, 1, 5, 6, glm::vec4(1, 0, 0, 1));
	physicsScene->addActor(box); */

	//PlaneClass* plane = new PlaneClass(glm::normalize(glm::vec2(1, 1)), -25);
	//physicsScene->addActor(plane);
	//
	//
	PlaneClass* plane2 = new PlaneClass(glm::normalize(glm::vec2(0, 1)), 0);
	physicsScene->addActor(plane2);

}

void DIYmain::SpringPhysicsTutorial()
{
	physicsScene = new DIYPhysicScene();
	physicsScene->collisionEnabled = true;
	physicsScene->timeStep = .016f;
	physicsScene->gravity = glm::vec2(0, -10);

	SphereClass* top_sphere = new SphereClass(glm::vec2(0, 40), glm::vec2(), 2, 2, glm::vec4(1, 1, 0, 1));
	top_sphere->is_static = true;
	physicsScene->addActor(top_sphere);

	SphereClass* collision_sphere = new SphereClass(glm::vec2(10, 25), glm::vec2(), 6, 6, glm::vec4(1, 1, 0, 1));
	collision_sphere->is_static = true;
	physicsScene->addActor(collision_sphere);

	const int CHAIN_LEN = 20;
	SphereClass* chain[CHAIN_LEN];
	SpringJoint* joints[CHAIN_LEN];

	float seperation = 2.5f;

	for (int i = 0; i < CHAIN_LEN; ++i)
	{
		float distance = seperation * (i + 1);
		chain[i] = new SphereClass(glm::vec2(distance, 40), glm::vec2(), 1.0f, 2, glm::vec4(1, 1, 0, 1));
		physicsScene->addActor(chain[i]);
	}

	float rest_dist = seperation;// sqrtf(seperation*seperation + seperation*seperation);

	SpringJoint* joint1 = new SpringJoint(top_sphere, chain[0], 550, 0.5f, rest_dist);
	physicsScene->addJoint(joint1);

	for (int i = 0; i < CHAIN_LEN - 1; ++i)
	{
		joints[i] = new SpringJoint(chain[i], chain[i + 1], 550, 0.5f, rest_dist);
		physicsScene->addJoint(joints[i]);
	}

}


void DIYmain::onUpdateRocket(float deltaTime)
{
	if (rocket != nullptr)
	{
		static float fireCounter = 0;
		fireCounter -= deltaTime;
		if (fireCounter <= 0)
		{
			float exhaustMass = .1f;
			fireCounter = 0.25;
			SphereClass *exhaust;
			glm::vec2 position = rocket->position;
			if (rocket->mass > exhaustMass)
			{
				rocket->mass -= exhaustMass;
				exhaust = new SphereClass(position, glm::vec2(0, 0), 1, exhaustMass, glm::vec4(0, 1, 0, 1));
				physicsScene->addActor(exhaust);
				exhaust->applyForceToActor(rocket, glm::vec2(1, 1));
			}
		}
	}
}
