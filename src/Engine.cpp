#include "Engine.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Debugging.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/Pipeline.hpp"

Engine::Engine(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
{
    // Create Window!
    CreateGLFWWindow();

    // Create Vulkan Instance!
    CreateVulkanInstance();

    // Create Device
    CreateDevice();

    // Create Pipeline
    CreatePipeline();
}

Engine::~Engine()
{
    m_Device.destroyPipeline(m_Pipeline);
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyRenderPass(m_RenderPass);
    for (vkInit::SwapChainFrame frame : m_SwapchainFrames)
        m_Device.destroyImageView(frame.imageView);
    m_Device.destroySwapchainKHR(m_Swapchain);
    m_Device.destroy(); 
    m_Instance.destroySurfaceKHR(m_Surface);
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

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &surface) != VK_SUCCESS)
        CONSOLE_ERROR("Failed to abstract the glfw surface for Vulkan.");
    else
        CONSOLE_DEBUG("Successfully abstracted the glfw surface for Vulkan.");
    m_Surface = surface;
}

void Engine::CreateDevice()
{
    m_PhysicalDevice = vkInit::ChoosePhysicalDevice(m_Instance);
    m_Device = vkInit::CreateLogicalDevice(m_PhysicalDevice, m_Surface);
    std::array<vk::Queue, 2> queues = vkInit::GetQueue(m_PhysicalDevice, m_Device, m_Surface);

    m_GraphicsQueue = queues[0];
    m_PresentQueue = queues[1];

    vkInit::SwapChainBundle bundle = vkInit::CreateSwapChain(m_Device, m_PhysicalDevice, m_Surface, m_Width, m_Height);
    m_Swapchain = bundle.swapchain;
    m_SwapchainFrames = bundle.frames;
    m_SwapchainFormat = bundle.format;
    m_SwapchainExtent = bundle.extent;
}

void Engine::CreatePipeline()
{
    vkInit::GraphicsPipelineInBundle specification{};
    specification.device = m_Device;
    specification.vertexShaderFilePath = PROJECT_DIR"/src/Shaders/TriangleVert.spv";
    specification.fragmentShaderFilePath = PROJECT_DIR"/src/Shaders/TriangleFrag.spv";
    specification.swapchainExtent = m_SwapchainExtent;
    specification.swapchainImageFormat = m_SwapchainFormat;

    vkInit::GraphicsPipelineOutBundle output = vkInit::MakeGraphicsPipeline(specification);
    m_PipelineLayout = output.layout;
    m_Pipeline = output.pipeline;
    m_RenderPass = output.renderpass;
}
