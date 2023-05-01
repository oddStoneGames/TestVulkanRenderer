#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Config.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>

namespace vkInit
{
    struct SwapChainFrame;
}

class Engine
{
public:
    Engine(uint32_t width = 1280, uint32_t height = 720);
    ~Engine();
    void RenderLoop();
private:
    void CreateGLFWWindow();
    void CreateVulkanInstance();
    void CreateDevice();
    void CreatePipeline();
    void FinalRenderingSetup();
    void DisplayFramerate();
    void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
private:
    // Window Properties and Window
    uint32_t m_Width, m_Height;
    GLFWwindow* m_Window{ nullptr };

    // Instance-Related Variables.
    vk::Instance m_Instance{ nullptr }; // Vulkan Instance
    vk::DebugUtilsMessengerEXT m_DebugMessenger{ nullptr }; // Debug Callback
    vk::DispatchLoaderDynamic m_Dldi; // Dynamic Instance Dispatcher
    vk::SurfaceKHR m_Surface; // Surface

    // Device-Related Variables.
    vk::PhysicalDevice m_PhysicalDevice{ nullptr }; // Vulkan Physical Device
    vk::Device m_Device{ nullptr }; // Vulkan Logical Device
    vk::Queue m_GraphicsQueue{ nullptr };  //Graphics Queue is the first queue from the graphics queue family.
    vk::Queue m_PresentQueue{ nullptr };
    vk::SwapchainKHR m_Swapchain{ nullptr };
    std::vector<vkInit::SwapChainFrame> m_SwapchainFrames;
    vk::Format m_SwapchainFormat;
    vk::Extent2D m_SwapchainExtent;

    // Pipeline-Related Variables.
    vk::PipelineLayout m_PipelineLayout;
    vk::RenderPass m_RenderPass;
    vk::Pipeline m_Pipeline;

    //Command-Related Variables
    vk::CommandPool m_CommandPool;
    vk::CommandBuffer m_MainCommandBuffer;

    //Synchronization-Related Variables
    vk::Fence m_InFlightFence;
    vk::Semaphore m_ImageAvailable, m_RenderComplete;
    double m_CurrentTime = 0.0, m_LastTime = 0.0;
    int m_NumFrames = 0;

    #ifdef NDEBUG
    const bool m_DebugMode = false;
    #else
    const bool m_DebugMode = true;
    #endif
};

#endif