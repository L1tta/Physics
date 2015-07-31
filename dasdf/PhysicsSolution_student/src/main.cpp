#include "Physics.h"
#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>
#include "DIYmain.h"

int main()
{
   Physics app;
   // DIYmain app;

    if (app.startup() == false)
    {
        return -1;
    }

    while (app.update() == true)
    {
        app.draw();
    }

    app.shutdown();

    return 0;
}