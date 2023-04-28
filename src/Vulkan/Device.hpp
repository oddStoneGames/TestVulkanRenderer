#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "../Config.hpp"

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

        CONSOLE_NEWLINE();
        CONSOLE_INFO("%s%s, %s", initString, properties.deviceName, deviceTypeString);
        CONSOLE_NEWLINE();
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
}

#endif // !DEVICE_HPP