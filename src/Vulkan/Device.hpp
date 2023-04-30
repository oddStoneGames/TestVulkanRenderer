#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "../Config.hpp"
#include "Debugging.hpp"

namespace vkInit
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct SwapChainBundle
    {
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> images;
        vk::Format format;
        vk::Extent2D extent;
    };

    inline void LogPhysicalDeviceProperties(const char* initString, const vk::PhysicalDevice& device)
    {
        vk::PhysicalDeviceProperties properties = device.getProperties();

        const char* deviceTypeString = nullptr;

        switch (properties.deviceType)
        {
        case (vk::PhysicalDeviceType::eCpu): deviceTypeString = "CPU"; break;
        case (vk::PhysicalDeviceType::eDiscreteGpu): deviceTypeString = "Discrete GPU"; break;
        case (vk::PhysicalDeviceType::eIntegratedGpu): deviceTypeString = "Integrated GPU"; break;
        case (vk::PhysicalDeviceType::eVirtualGpu): deviceTypeString = "Virtual GPU"; break;
        default: deviceTypeString = "Other"; break;
        }

        CONSOLE_INFO("%s%s, %s", initString, static_cast<const char*>(properties.deviceName), deviceTypeString);
    }

    inline bool CheckPhysicalDeviceExtenionSupport(const vk::PhysicalDevice& device, const std::vector<const char*> extensions)
    {
        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties())
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    inline bool IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device)
    {
        const std::vector<const char*> requestedExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        return CheckPhysicalDeviceExtenionSupport(device, requestedExtensions);
    }

    inline vk::PhysicalDevice ChoosePhysicalDevice(vk::Instance& instance)
    {
        std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

        for (const auto& device : availableDevices)
        {
            if (IsPhysicalDeviceSuitable(device))
            {
                LogPhysicalDeviceProperties("Choosing Physical Device: ", device);
                return device;
            }
        }
        
        return nullptr;
    }

    inline QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
        CONSOLE_DEBUG("Available Queue Families on Selected Physical Device: %d", queueFamilies.size());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsFamily = i;
                indices.presentFamily = i;
                CONSOLE_DEBUG("Queue Family %d is suitable for graphics.", i);
            }

            if (device.getSurfaceSupportKHR(i, surface))
            {
                indices.presentFamily = i;
                CONSOLE_DEBUG("Queue Family %d is suitable for presenting.", i);
            }

            if (indices.isComplete())
                break;

            i++;
        }

        return indices;
    }

    vk::Device CreateLogicalDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR surface)
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
        std::vector<uint32_t> uniqueIndices;
        uniqueIndices.push_back(indices.graphicsFamily.value());
        if (indices.graphicsFamily.value() != indices.presentFamily.value())
            uniqueIndices.push_back(indices.presentFamily.value());

        float queuePriority = 1.0f;
        
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
        for (uint32_t queueFamilyIndex : uniqueIndices)
        {
            queueCreateInfo.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority));
        }

        vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
        
        const std::vector<const char*> layers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> extensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
            vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfo.size()), queueCreateInfo.data(), static_cast<uint32_t>(layers.size()), layers.data(),
            static_cast<uint32_t>(extensions.size()), extensions.data(), &deviceFeatures
        );

        try
        {
            vk::Device device = physicalDevice.createDevice(deviceInfo);
            CONSOLE_INFO("GPU has been successfully abstraced!");

            return device;
        }
        catch (vk::SystemError err)
        {
            CONSOLE_ERROR("Failed to Create VKDevice! %s", err.what());
            return nullptr;
        }
    }

    std::array<vk::Queue, 2> GetQueue(const vk::PhysicalDevice& physicalDevice, const vk::Device& device, const vk::SurfaceKHR surface)
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

        return {
            device.getQueue(indices.graphicsFamily.value(), 0),
            device.getQueue(indices.presentFamily.value(), 0)
        };
    }

    inline SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
    {
        SwapChainSupportDetails support;

        support.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

        CONSOLE_DEBUG("Minimum Image Count: %d", support.capabilities.minImageCount);
        CONSOLE_DEBUG("Maximum Image Count: %d", support.capabilities.maxImageCount);

        CONSOLE_DEBUG("Current Image Extent: ");
        CONSOLE_DEBUG("\t Width: %d", support.capabilities.currentExtent.width);
        CONSOLE_DEBUG("\t Height: %d", support.capabilities.currentExtent.height);

        CONSOLE_DEBUG("Minimum Image Extent: ");
        CONSOLE_DEBUG("\t Width: %d", support.capabilities.minImageExtent.width);
        CONSOLE_DEBUG("\t Height: %d", support.capabilities.minImageExtent.height);

        CONSOLE_DEBUG("Maximum Image Extent: ");
        CONSOLE_DEBUG("\t Width: %d", support.capabilities.maxImageExtent.width);
        CONSOLE_DEBUG("\t Height: %d", support.capabilities.maxImageExtent.height);

        CONSOLE_DEBUG("Maxmium Image Array Layers: %d", support.capabilities.maxImageArrayLayers);

        #ifndef NDEBUG

        CONSOLE_DEBUG("Supported Transforms: ");
        std::vector<std::string> stringList = LogTransformBits(support.capabilities.supportedTransforms);
        for(std::string line : stringList)
            CONSOLE_DEBUG("\t %s", line.c_str());

        CONSOLE_DEBUG("Current Transform: ");
        stringList = LogTransformBits(support.capabilities.currentTransform);
        for (std::string line : stringList)
            CONSOLE_DEBUG("\t %s", line.c_str());

        CONSOLE_DEBUG("Supported Alpha Operations: ");
        stringList = LogAlphaCompositeBits(support.capabilities.supportedCompositeAlpha);
        for (std::string line : stringList)
            CONSOLE_DEBUG("\t %s", line.c_str());

        CONSOLE_DEBUG("Supported Image Usage: ");
        stringList = LogImageUsageBits(support.capabilities.supportedUsageFlags);
        for (std::string line : stringList)
            CONSOLE_DEBUG("\t %s", line.c_str());

        #endif // !NDEBUG

        support.formats = physicalDevice.getSurfaceFormatsKHR(surface);

        for (vk::SurfaceFormatKHR supportedFormat : support.formats)
        {
            CONSOLE_DEBUG("Supported Pixel Format: %s", vk::to_string(supportedFormat.format).c_str());
            CONSOLE_DEBUG("Supported Color Space: %s", vk::to_string(supportedFormat.colorSpace).c_str());
        }

        support.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
        CONSOLE_DEBUG("Supported Present Modes: ");
        for(vk::PresentModeKHR presentMode : support.presentModes)
            CONSOLE_DEBUG("\t %s", LogPresentMode(presentMode).c_str());

        return support;
    }

    vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> formats)
    {
        for (vk::SurfaceFormatKHR format : formats)
            if(format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return format;

        return formats[0];
    }

    vk::PresentModeKHR ChooseSwapChainPresentMode(std::vector<vk::PresentModeKHR> presentModes)
    {
        for(vk::PresentModeKHR presentMode : presentModes)
            if (presentMode == vk::PresentModeKHR::eMailbox)
                return presentMode;

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D ChooseSwapChainExtent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            vk::Extent2D extent = { width, height };

            extent.width = std::min(capabilities.maxImageExtent.width, std::max(capabilities.minImageExtent.width, width));
            extent.height = std::min(capabilities.maxImageExtent.height, std::max(capabilities.minImageExtent.height, height));

            return extent;
        }
    }

    SwapChainBundle CreateSwapChain(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, int width, int height)
    {
        SwapChainSupportDetails support = QuerySwapChainSupport(physicalDevice, surface);

        vk::SurfaceFormatKHR format = ChooseSwapChainSurfaceFormat(support.formats);
        vk::PresentModeKHR presentMode = ChooseSwapChainPresentMode(support.presentModes);
        vk::Extent2D extent = ChooseSwapChainExtent(width, height, support.capabilities);

        uint32_t imageCount = std::min(support.capabilities.maxImageCount, support.capabilities.minImageCount + 1);

        vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
            vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format, format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment
        );

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily.value() != indices.presentFamily.value())
        {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }

        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

        SwapChainBundle bundle{};
        
        try
        {
            bundle.swapchain = device.createSwapchainKHR(createInfo);
            CONSOLE_INFO("Swapchain created successfully!");
        }
        catch (vk::SystemError err)
        {
            CONSOLE_ERROR("Failed to Create Swapchain! %s", err.what());
            return bundle;
        }

        bundle.images = device.getSwapchainImagesKHR(bundle.swapchain);
        bundle.format = format.format;
        bundle.extent = extent;

        return bundle;
    }
}

#endif // !DEVICE_HPP