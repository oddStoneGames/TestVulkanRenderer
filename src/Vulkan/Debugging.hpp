#ifndef DEBUGGING_HPP
#define DEBUGGING_HPP

#include "../Config.hpp"

namespace vkInit
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            CONSOLE_TRACE("%s", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            CONSOLE_INFO("%s", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            CONSOLE_WARN("%s", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            CONSOLE_ERROR("%s", pCallbackData->pMessage); break;
        default: break;
        }

        return VK_FALSE;
    }

    inline vk::DebugUtilsMessengerEXT CreateDebugMessenger(vk::Instance& instance, vk::DispatchLoaderDynamic& dldi)
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugCallback,
            nullptr
        );

        return instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
    }

    inline std::vector<std::string> LogTransformBits(vk::SurfaceTransformFlagsKHR bits)
    {
        std::vector<std::string> result;

        if (bits & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            result.push_back("Identity");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate90)
            result.push_back("90 Degree Rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate180)
            result.push_back("180 Degree Rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eRotate270)
            result.push_back("270 Degree Rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror)
            result.push_back("Horizontal Mirror");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90)
            result.push_back("Horizontal Mirror, then 90 degree rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180)
            result.push_back("Horizontal Mirror, then 180 degree rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270)
            result.push_back("Horizontal Mirror, then 270 degree rotation");

        if (bits & vk::SurfaceTransformFlagBitsKHR::eInherit)
            result.push_back("Inherit");

        return result;
    }

    inline std::vector<std::string> LogAlphaCompositeBits(vk::CompositeAlphaFlagsKHR bits)
    {
        std::vector<std::string> result;

        if (bits & vk::CompositeAlphaFlagBitsKHR::eOpaque)
            result.push_back("Opaque (Alpha Ignored)");

        if (bits & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            result.push_back("Pre multiplied(alpha expected to be already be multiplied in image)");

        if (bits & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            result.push_back("Post multiplied(alpha will be applied during composition)");

        if (bits & vk::CompositeAlphaFlagBitsKHR::eInherit)
            result.push_back("Inherited");

        return result;
    }

    inline std::vector<std::string> LogImageUsageBits(vk::ImageUsageFlags bits)
    {
        std::vector<std::string> result;

        if (bits & vk::ImageUsageFlagBits::eTransferSrc)
            result.push_back("Transfer src: image can be used as the source of a transfer command.");

        if (bits & vk::ImageUsageFlagBits::eTransferDst)
            result.push_back("Transfer dst: image can be used as the destination of a transfer command.");

        if (bits & vk::ImageUsageFlagBits::eSampled)
            result.push_back("Sampled: specifies that the image can be used to create a VkImageView suitable for occupying a \
VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE or VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, \
and be sampled by a shader.");

        if (bits & vk::ImageUsageFlagBits::eStorage)
            result.push_back("Storage: specifies that the image can be used to create a VkImageView \
suitable for occupying a VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.");

        if (bits & vk::ImageUsageFlagBits::eColorAttachment)
            result.push_back("Color Attachment: specifies that the image can be used to create a VkImageView \
suitable for use as a color or resolve attachment in a VkFramebuffer.");

        if (bits & vk::ImageUsageFlagBits::eDepthStencilAttachment)
            result.push_back("Depth Stencil Attachment: specifies that the image can be used to create a VkImageView \
suitable for use as a depth/stencil or depth/stencil resolve attachment in a VkFramebuffer.");

        if (bits & vk::ImageUsageFlagBits::eTransientAttachment)
            result.push_back("Transient Attachment: specifies that implementations may support using memory allocations \
with the VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT to back an image with this usage. \
This bit can be set for any image that can be used to create a VkImageView suitable for use as a color, \
resolve, depth/stencil, or input attachment.");

        if (bits & vk::ImageUsageFlagBits::eInputAttachment)
            result.push_back("Input Attachment: specifies that the image can be used to create a VkImageView \
suitable for occupying VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; \
be read from a shader as an input attachment; and be used as an input attachment in a framebuffer.");

        if (bits & vk::ImageUsageFlagBits::eFragmentDensityMapEXT)
            result.push_back("Fragment Density Map: specifies that the image can be used to create a VkImageView \
suitable for use as a fragment density map image.");

        if (bits & vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR)
            result.push_back("Fragment Shading Rate Attachment: specifies that the image can be used to create a VkImageView \
suitable for use as a fragment shading rate attachment or shading rate image");

//         if (bits & vk::ImageUsageFlagBits::eVideoDecodeDstKHR)
//             result.push_back("Video Decode Destination: specifies that the image can be used as a \
// decode output picture in a video decode operation.");

//         if (bits & vk::ImageUsageFlagBits::eVideoDecodeSrcKHR)
//             result.push_back("Video Decode Source: reserved for future use.");

//         if (bits & vk::ImageUsageFlagBits::eVideoDecodeDpbKHR)
//             result.push_back("Video Decode Dpb: specifies that the image can be used as an \
// output reconstructed picture or an input reference picture in a video decode operation.");

//         if (bits & vk::ImageUsageFlagBits::eAttachmentFeedbackLoopEXT)
//             result.push_back("Feedback Loop Attachment: specifies that the image can be transitioned \
// to the VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT layout to be used as a color \
// or depth/stencil attachment in a VkFramebuffer and/or as a read-only input resource in a \
// shader (sampled image, combined image sampler or input attachment) in the same render pass.");

        return result;
    }

    inline std::string LogPresentMode(vk::PresentModeKHR presentMode)
    {
        std::string str{};

        if (presentMode == vk::PresentModeKHR::eImmediate)
            str = "Immediate: specifies that the presentation engine does not wait for a vertical blanking period \
to update the current image, meaning this mode may result in visible tearing. \
No internal queuing of presentation requests is needed, as the requests are applied immediately.";

        if (presentMode == vk::PresentModeKHR::eMailbox)
            str = "Mail box: specifies that the presentation engine waits for the next vertical blanking period \
to update the current image. Tearing cannot be observed. An internal single-entry queue is used to hold pending presentation requests. \
If the queue is full when a new presentation request is received, the new request replaces the existing entry, \
and any images associated with the prior entry become available for re-use by the application. \
One request is removed from the queue and processed during each vertical blanking period in which the queue is non-empty.";

        if (presentMode == vk::PresentModeKHR::eFifo)
            str = "Fifo: specifies that the presentation engine waits for the next vertical blanking period \
to update the current image. Tearing cannot be observed. An internal queue is used to hold pending presentation requests. \
New requests are appended to the end of the queue, and one request is removed from the beginning of the queue \
and processed during each vertical blanking period in which the queue is non-empty. \
This is the only value of presentMode that is required to be supported.";

        if (presentMode == vk::PresentModeKHR::eFifoRelaxed)
            str = "Fifo Relaxed: specifies that the presentation engine generally waits for the next vertical blanking period \
to update the current image. If a vertical blanking period has already passed since the last update of the current image \
then the presentation engine does not wait for another vertical blanking period for the update, meaning this mode may result \
in visible tearing in this case. This mode is useful for reducing visual stutter with an application that will mostly present \
a new image before the next vertical blanking period, but may occasionally be late, and present a new image just after the next \
vertical blanking period. An internal queue is used to hold pending presentation requests. New requests are appended to the end \
of the queue, and one request is removed from the beginning of the queue and processed during or after each vertical blanking period \
in which the queue is non-empty.";

        if (presentMode == vk::PresentModeKHR::eSharedDemandRefresh)
            str = "Shared Demand Refresh: specifies that the presentation engine and application have concurrent access to a single image, \
which is referred to as a shared presentable image. The presentation engine is only required to update the current image after a new \
presentation request is received. Therefore the application must make a presentation request whenever an update is required. However, \
the presentation engine may update the current image at any point, meaning this mode may result in visible tearing.";

        if (presentMode == vk::PresentModeKHR::eSharedContinuousRefresh)
            str = "Shared Continuous Refresh: specifies that the presentation engine and application have concurrent access to a single image, \
which is referred to as a shared presentable image. The presentation engine periodically updates the current image on its regular refresh cycle. \
The application is only required to make one initial presentation request, after which the presentation engine must update the current image \
without any need for further presentation requests. The application can indicate the image contents have been updated by making a presentation \
request, but this does not guarantee the timing of when it will be updated. This mode may result in visible tearing if rendering to the image \
is not timed correctly.";

        return str;
    }
}

#endif // !DEBUGGING_HPP
