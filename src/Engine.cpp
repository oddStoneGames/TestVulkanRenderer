#include "Engine.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Debugging.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Framebuffer.hpp"
#include "Vulkan/Command.hpp"
#include "Vulkan/Sync.hpp"
#include <limits>

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

    // Do all the other things like
    // create framebuffers, command pool, use synchronization.
    FinalRenderingSetup();
}

Engine::~Engine()
{
    m_Device.waitIdle();

    m_Device.destroyFence(m_InFlightFence);
    m_Device.destroySemaphore(m_ImageAvailable);
    m_Device.destroySemaphore(m_RenderComplete);
    m_Device.destroyCommandPool(m_CommandPool);
    m_Device.destroyPipeline(m_Pipeline);
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyRenderPass(m_RenderPass);
    for (const vkInit::SwapChainFrame& frame : m_SwapchainFrames)
    {
        m_Device.destroyImageView(frame.imageView);
        m_Device.destroyFramebuffer(frame.framebuffer);
    }
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

void Engine::FinalRenderingSetup()
{
    vkInit::FramebufferInput input;
    input.device = m_Device;
    input.renderPass = m_RenderPass;
    input.swapchainextent = m_SwapchainExtent;
    vkInit::CreateFramebuffers(input, m_SwapchainFrames);

    m_CommandPool = vkInit::CreateCommandPool(m_Device, m_PhysicalDevice, m_Surface);

    vkInit::CommandBufferInputChunk commandBufferInput = { m_Device, m_CommandPool, m_SwapchainFrames };
    m_MainCommandBuffer = vkInit::CreateCommandBuffers(commandBufferInput);

    m_InFlightFence = vkInit::CreateFence(m_Device);
    m_ImageAvailable = vkInit::CreateSemaphore(m_Device);
    m_RenderComplete = vkInit::CreateSemaphore(m_Device);
}

void Engine::RenderLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        DisplayFramerate();

        m_Device.waitForFences(1, &m_InFlightFence, VK_TRUE, UINT32_MAX);
        m_Device.resetFences(1, &m_InFlightFence);

        uint32_t imageIndex =  m_Device.acquireNextImageKHR(m_Swapchain, UINT32_MAX, m_ImageAvailable, nullptr).value;
        vk::CommandBuffer commandBuffer = m_SwapchainFrames[imageIndex].commandBuffer;
        commandBuffer.reset();
        RecordDrawCommands(commandBuffer, imageIndex);

        vk::SubmitInfo submitInfo{};
        vk::Semaphore waitSemaphores[] = { m_ImageAvailable };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        
        vk::Semaphore signalSemaphores[] = { m_RenderComplete };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        try
        {
            m_GraphicsQueue.submit(submitInfo, m_InFlightFence);
        }
        catch (const vk::SystemError& err)
        {
            CONSOLE_ERROR("Failed to submit commands to the graphics queue! %s", err.what());
        }

        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        vk::SwapchainKHR swapchains[] = { m_Swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        m_PresentQueue.presentKHR(presentInfo);
    }
}

void Engine::DisplayFramerate()
{
    m_CurrentTime = glfwGetTime();
    double delta = m_CurrentTime - m_LastTime;

    if (delta >= 1)
    {
        int framerate{ std::max(1, int(m_NumFrames / delta)) };
        std::stringstream title;
        title << "Running at " << framerate << " fps.";
        glfwSetWindowTitle(m_Window, title.str().c_str());
        m_LastTime = m_CurrentTime;
        m_NumFrames = -1;
    }

    m_NumFrames++;
}

void Engine::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo{};

    try
    {
        commandBuffer.begin(beginInfo);
    }
    catch (const vk::SystemError& err)
    {
        CONSOLE_ERROR("Failed to begin recording command buffer! %s", err.what());
    }

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapchainFrames[imageIndex].framebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = m_SwapchainExtent;
    vk::ClearValue clearColor = { std::array<float, 4>{ 0.02f, 0.04f, 0.08f, 1.0f} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
    commandBuffer.draw(3, 1, 0, 0);
    commandBuffer.endRenderPass();
    
    try
    {
        commandBuffer.end();
    }
    catch (const vk::SystemError& err)
    {
        CONSOLE_ERROR("Failed to finish recording command buffer! %s", err.what());
    }
}