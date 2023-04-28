#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>

class Engine
{
public:
    Engine(uint32_t width = 1280, uint32_t height = 720);
    ~Engine();
private:
    GLFWwindow* m_Window;
    uint32_t m_Width, m_Height;
};

#endif