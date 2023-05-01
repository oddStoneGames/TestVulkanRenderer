#ifndef ENTRYPOINT_CPP
#define ENTRYPOINT_CPP

#include "Engine.hpp"

int main()
{
    // Create Vulkan Engine.
    Engine vulkanEngine;
    
    // Create a default scene.
    Scene scene;

    // Start the Render Loop!
    vulkanEngine.RenderLoop(&scene);

    return 0;
}

#endif