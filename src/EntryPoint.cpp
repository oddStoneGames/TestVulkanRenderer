#ifndef ENTRYPOINT_CPP
#define ENTRYPOINT_CPP

#include "Engine.hpp"

int main()
{
    // Create Vulkan Engine.
    Engine vulkanEngine;
    
    // Start the Render Loop!
    vulkanEngine.RenderLoop();

    return 0;
}

#endif