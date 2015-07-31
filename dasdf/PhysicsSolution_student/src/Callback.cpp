#include "Callback.h"
#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include <cstdio>

Callback::Callback()
{
	Colour1 = vec4(1, 1, 1, 1);
	Colour2 = vec4(1, 1, 1, 1);
	trigger = false;
	trigger2 = false;
}
Callback::~Callback(){}

void Callback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	printf("onContact");
}

void Callback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
	printf("onConstraintBreak");
}

void Callback::onWake(PxActor** actors, PxU32 count)
{
	printf("onWake");

}

void Callback::onSleep(PxActor** actors, PxU32 count)
{
	printf("onSleep");

}

void Callback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eDELETED_SHAPE_TRIGGER | PxTriggerPairFlag::eDELETED_SHAPE_OTHER))
			continue;
		if ((pairs[i].otherShape->getActor() == getActor1) && (pairs[i].triggerShape->getActor() == getActor2))
		{
			if (trigger == false)
			{
				Colour1 = vec4(1, 0, 0, 1);
				trigger = true;
			}
			else if (trigger == true)
			{
				Colour1 = vec4(1, 1, 1, 1);
				trigger = false;
			}
			
		}
	}

	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eDELETED_SHAPE_TRIGGER | PxTriggerPairFlag::eDELETED_SHAPE_OTHER))
			continue;
		if ((pairs[i].otherShape->getActor() == getActor3) && (pairs[i].triggerShape->getActor() == getActor2))
		{
			if (trigger2 == false)
			{
				Colour2 = vec4(0, 0, 1, 1);
				trigger2 = true;
			}
			else if (trigger2 == true)
			{
				Colour2 = vec4(1, 1, 1, 1);
				trigger2 = false;
			}
		}
	}
	
}

