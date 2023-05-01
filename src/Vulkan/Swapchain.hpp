#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "../Config.hpp"
#include "Debugging.hpp"
#include "QueueFamily.hpp"
#include "Frame.hpp"

namespace vkInit
{
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct SwapChainBundle
    {
        vk::SwapchainKHR swapchain;
        std::vector<SwapChainFrame> frames;
        vk::Format format;
        vk::Extent2D extent;
    };

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
        for (std::string line : stringList)
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
        for (vk::PresentModeKHR presentMode : support.presentModes)
            CONSOLE_DEBUG("\t %s", LogPresentMode(presentMode).c_str());

        return support;
    }

    vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> formats)
    {
        for (vk::SurfaceFormatKHR format : formats)
            if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return format;

        return formats[0];
    }

    vk::PresentModeKHR ChooseSwapChainPresentMode(std::vector<vk::PresentModeKHR> presentModes)
    {
        for (vk::PresentModeKHR presentMode : presentModes)
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

        std::vector<vk::Image> images = device.getSwapchainImagesKHR(bundle.swapchain);
        bundle.frames.resize(images.size());

        for (size_t i = 0; i < images.size(); i++)
        {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = images[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.format = format.format;
            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            bundle.frames[i].image = images[i];
            bundle.frames[i].imageView = device.createImageView(createInfo);
        }

        bundle.format = format.format;
        bundle.extent = extent;

        return bundle;
    }
}

#endif