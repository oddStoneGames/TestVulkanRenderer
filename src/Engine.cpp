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

    // Create Assets
    CreateAssets();
}

Engine::~Engine()
{
    m_Device.waitIdle();

    m_TriangleMesh->Destroy();
    DestroySwapchain();
    m_Device.destroyCommandPool(m_CommandPool);
    m_Device.destroyPipeline(m_Pipeline);
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyRenderPass(m_RenderPass);
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

    CreateSwapchain();
    m_FrameNumber = 0;
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

void Engine::CreateSwapchain()
{
    vkInit::SwapChainBundle bundle = vkInit::CreateSwapChain(m_Device, m_PhysicalDevice, m_Surface, m_Width, m_Height);
    m_Swapchain = bundle.swapchain;
    m_SwapchainFrames = bundle.frames;
    m_SwapchainFormat = bundle.format;
    m_SwapchainExtent = bundle.extent;
    m_MaxFramesInFlight = static_cast<int>(m_SwapchainFrames.size());
}

void Engine::RecreateSwapchain()
{
    m_Width = 0;
    m_Height = 0;

    while (m_Width == 0 || m_Height == 0)
    {
        glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
        glfwWaitEvents();
    }

    m_Device.waitIdle();
    DestroySwapchain();

    CreateSwapchain();
    CreateFramebuffers();
    CreateSyncObjects();
    vkInit::CommandBufferInputChunk commandBufferInput = { m_Device, m_CommandPool, m_SwapchainFrames };
    vkInit::CreateFrameCommandBuffers(commandBufferInput);
}

void Engine::DestroySwapchain()
{
    for (const vkInit::SwapChainFrame& frame : m_SwapchainFrames)
    {
        m_Device.destroyImageView(frame.imageView);
        m_Device.destroyFramebuffer(frame.framebuffer);
        m_Device.destroyFence(frame.inFlight);
        m_Device.destroySemaphore(frame.imageAvailable);
        m_Device.destroySemaphore(frame.renderComplete);
        m_Device.freeCommandBuffers(m_CommandPool, frame.commandBuffer);
    }
    m_Device.destroySwapchainKHR(m_Swapchain);
}

void Engine::CreateFramebuffers()
{
    vkInit::FramebufferInput input;
    input.device = m_Device;
    input.renderPass = m_RenderPass;
    input.swapchainextent = m_SwapchainExtent;
    vkInit::CreateFramebuffers(input, m_SwapchainFrames);
}

void Engine::CreateSyncObjects()
{
    for (vkInit::SwapChainFrame& frame : m_SwapchainFrames)
    {
        frame.inFlight = vkInit::CreateFence(m_Device);
        frame.imageAvailable = vkInit::CreateSemaphore(m_Device);
        frame.renderComplete = vkInit::CreateSemaphore(m_Device);
    }
}

void Engine::FinalRenderingSetup()
{
    CreateFramebuffers();

    m_CommandPool = vkInit::CreateCommandPool(m_Device, m_PhysicalDevice, m_Surface);

    vkInit::CommandBufferInputChunk commandBufferInput = { m_Device, m_CommandPool, m_SwapchainFrames };
    m_MainCommandBuffer = vkInit::CreateCommandBuffer(commandBufferInput);
    vkInit::CreateFrameCommandBuffers(commandBufferInput);

    CreateSyncObjects();
}

void Engine::RenderLoop(Scene* scene)
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        DisplayFramerate();

        m_Device.waitForFences(1, &m_SwapchainFrames[m_FrameNumber].inFlight, VK_TRUE, UINT32_MAX);

        uint32_t imageIndex = -1;
        try
        {
            vk::ResultValue acquire = m_Device.acquireNextImageKHR(m_Swapchain, UINT32_MAX, m_SwapchainFrames[m_FrameNumber].imageAvailable, nullptr);
            imageIndex = acquire.value;
        }
        catch (const vk::OutOfDateKHRError& err)
        {
            CONSOLE_INFO("Recreate Swapchain!");
            RecreateSwapchain();
            continue;
        }
        m_Device.resetFences(1, &m_SwapchainFrames[m_FrameNumber].inFlight);

        vk::CommandBuffer commandBuffer = m_SwapchainFrames[m_FrameNumber].commandBuffer;
        commandBuffer.reset();
        RecordDrawCommands(commandBuffer, imageIndex, scene);

        vk::SubmitInfo submitInfo{};
        vk::Semaphore waitSemaphores[] = { m_SwapchainFrames[m_FrameNumber].imageAvailable };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        
        vk::Semaphore signalSemaphores[] = { m_SwapchainFrames[m_FrameNumber].renderComplete };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        try
        {
            m_GraphicsQueue.submit(submitInfo, m_SwapchainFrames[m_FrameNumber].inFlight);
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

        vk::Result present;
        try
        {
            present = m_PresentQueue.presentKHR(presentInfo);
        }
        catch (const vk::OutOfDateKHRError& err)
        {
            present = vk::Result::eErrorOutOfDateKHR;
        }
        
        if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR)
        {
            CONSOLE_INFO("Recreate Swapchain!");
            RecreateSwapchain();
            continue;
        }

        m_FrameNumber = (m_FrameNumber + 1) % m_MaxFramesInFlight;
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

void Engine::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene)
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

    PrepareScene(commandBuffer);

    for (glm::vec3 pos : scene->trianglePositions)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        vkInit::Constants constant;
        constant.model = model;
        commandBuffer.pushConstants(m_PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(constant), &constant);
        commandBuffer.draw(3, 1, 0, 0);
    }
    
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

void Engine::CreateAssets()
{
    m_TriangleMesh = std::make_unique<TriangleMesh>(m_Device, m_PhysicalDevice);
}

void Engine::PrepareScene(vk::CommandBuffer commandBuffer)
{
    vk::Buffer vertexBuffers[] = { m_TriangleMesh->vertexBuffer.buffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
}