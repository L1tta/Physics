#ifndef _DIYMAIN_H_
#define _DIYMAIN_H_
#include "Application.h"
#include "Camera.h"

#include "GLFW/glfw3.h"
#include "gl_core_4_4.h"
#include "Gizmos.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <iostream>
#include "DIYPhysicsEngine.h"

class DIYmain : public Application
{
public:
	DIYmain();
	virtual ~DIYmain();
	virtual bool startup();
	virtual void shutdown();
	virtual bool update();
	virtual void draw();

	void DIYPhysicsRocketSetup();
	void upDate2DPhysics(float delta);
	void DIYPhysicsCollisionTutorial();
	void draw2DGizmo();
	void onUpdateRocket(float deltaTime);
	void SpringPhysicsTutorial();
	glm::vec2 GetWorldMouse();

	DIYPhysicScene* physicsScene;
	SphereClass* rocket;
	GLFWwindow* window;

	float prevTime;
	float AR;

	FlyCamera m_camera;
};

#endif //_DIYMAIN_H_