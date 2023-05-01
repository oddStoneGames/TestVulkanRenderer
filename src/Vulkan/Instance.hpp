#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "../Config.hpp"

namespace vkInit
{
    // Returns true if the Current Instance has all the Layers that we request.
    inline bool CheckExtensionSupport(std::vector<const char*> extensions)
    {
        std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

        CONSOLE_NEWLINE();
        CONSOLE_DEBUG("Available Extensions: ");
        for (const auto& extn : availableExtensions)
            CONSOLE_DEBUG("\t %s", static_cast<const char*>(extn.extensionName));

        for (const char* extension : extensions)
        {
            bool found = false;
            for (const auto& extensionProps : availableExtensions)
            {
                if (strcmp(extension, extensionProps.extensionName) == 0)
                {
                    CONSOLE_INFO("Requested extension \"%s\" is supported.", extension);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                CONSOLE_ERROR("Requested extension \"%s\" not supported.", extension);
                return false;
            }
        }

        return true;
    }

    // Returns true if the Current Instance has all the Layers that we request.
    inline bool CheckLayerSupport(std::vector<const char*> layers)
    {
        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        CONSOLE_NEWLINE();
        CONSOLE_DEBUG("Available Layers: ");
        for (const auto& layerProps : availableLayers)
            CONSOLE_DEBUG("\t %s", static_cast<const char*>(layerProps.layerName));

        for (const char* layer : layers)
        {
            bool found = false;
            for (const auto& layerProps : availableLayers)
            {
                if (strcmp(layer, layerProps.layerName) == 0)
                {
                    CONSOLE_INFO("Requested layer \"%s\" is supported.", layer);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                CONSOLE_ERROR("Requested layer \"%s\" not supported.", layer);
                return false;
            }
        }

        return true;
    }

    inline vk::Instance CreateInstance(const char* applicationName, bool debug)
	{
        uint32_t version = vk::enumerateInstanceVersion();

        CONSOLE_DEBUG("System can support upto Vulkan %d.%d.%d", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), 
            VK_API_VERSION_PATCH(version));

        // To make sure compatibility with more devices.
        version = VK_MAKE_API_VERSION(0, 1, 0, 0);

        vk::ApplicationInfo appInfo("Vulkan Renderer", version, "Odd Engine", version, version);

        vk::InstanceCreateInfo createInfo;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (debug)
            extensions.push_back("VK_EXT_debug_utils");

        if (!CheckExtensionSupport(extensions))
            return nullptr;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        const std::vector<const char*> validationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

        if (debug && !CheckLayerSupport(validationLayers))
            return nullptr;

        createInfo.enabledLayerCount = debug ? static_cast<uint32_t>(validationLayers.size()) : 0;
        createInfo.ppEnabledLayerNames = debug ? validationLayers.data() : nullptr;

        try
        {
            return vk::createInstance(createInfo);
        }
        catch (const vk::SystemError& err)
        {
            CONSOLE_ERROR("Failed to Create VKInstance! %s", err.what());
            return nullptr;
        }
	}
}

#endif // !INSTANCE_HPP