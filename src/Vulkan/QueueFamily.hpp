#ifndef QUEUEFAMILY_HPP
#define QUEUEFAMILY_HPP

#include "../Config.hpp"

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

    std::array<vk::Queue, 2> GetQueue(const vk::PhysicalDevice& physicalDevice, const vk::Device& device, const vk::SurfaceKHR surface)
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

        return {
            device.getQueue(indices.graphicsFamily.value(), 0),
            device.getQueue(indices.presentFamily.value(), 0)
        };
    }
}

#endif