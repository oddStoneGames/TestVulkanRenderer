#ifndef FRAME_HPP
#define FRAME_HPP

#include "../Config.hpp"

namespace vkInit
{
    struct SwapChainFrame
    {
        vk::Image image;
        vk::ImageView imageView;
        vk::Framebuffer framebuffer;
        vk::CommandBuffer commandBuffer;
    };
}

#endif