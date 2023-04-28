#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Config.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>

class Engine
{
public:
    Engine(uint32_t width = 1280, uint32_t height = 720);
    ~Engine();
private:
    void CreateGLFWWindow();
    void CreateVulkanInstance();
    void CreatePhysicalDevice();
private:
    // Window Properties and Window
    uint32_t m_Width, m_Height;
    GLFWwindow* m_Window{ nullptr };

    // Instance-Related Variables.
    vk::Instance m_Instance{ nullptr }; // Vulkan Instance
    vk::DebugUtilsMessengerEXT m_DebugMessenger{ nullptr }; // Debug Callback
    vk::DispatchLoaderDynamic m_Dldi; // Dynamic Instance Dispatcher

    vk::PhysicalDevice m_PhysicalDevice; // Vulkan Physical Device


    #ifdef NDEBUG
    const bool m_DebugMode = false;
    #else
    const bool m_DebugMode = true;
    #endif
};

#endif