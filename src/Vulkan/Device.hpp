#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "../Config.hpp"
#include "QueueFamily.hpp"

namespace vkInit
{
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
}

#endif // !DEVICE_HPP