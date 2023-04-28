#include "Engine.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Debugging.hpp"
#include "Vulkan/Device.hpp"

Engine::Engine(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
{
    // Create Window!
    CreateGLFWWindow();

    // Create Vulkan Instance!
    CreateVulkanInstance();

    // Create Physical Device
    CreatePhysicalDevice();
}

Engine::~Engine()
{
    if(m_DebugMode)
        m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
    m_Instance.destroy();

    if(m_Window) glfwDestroyWindow(m_Window);

    CONSOLE_INFO("Terminating GLFW!");
    glfwTerminate();
}

void Engine::CreateGLFWWindow()
{
    CONSOLE_INFO("Intializing GLFW!");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_Width, m_Height, "Vulkan Renderer", nullptr, nullptr);

    if (!m_Window)
        CONSOLE_ERROR("Failed to Create GLFW Window!");
    else
        CONSOLE_INFO("GLFW Window Created Successfully!");
}

void Engine::CreateVulkanInstance()
{
    m_Instance = vkInit::CreateInstance("Vulkan Renderer", m_DebugMode);
    m_Dldi = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);
    if (m_DebugMode)
        m_DebugMessenger = vkInit::CreateDebugMessenger(m_Instance, m_Dldi);
}

void Engine::CreatePhysicalDevice()
{
    m_PhysicalDevice = vkInit::ChoosePhysicalDevice(m_Instance);
}
