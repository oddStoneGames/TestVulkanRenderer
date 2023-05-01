#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "../Config.hpp"
#include "Frame.hpp"

namespace vkInit
{
	struct FramebufferInput
	{
		vk::Device device;
		vk::RenderPass renderPass;
		vk::Extent2D swapchainextent;
	};

	void CreateFramebuffers(FramebufferInput input, std::vector<SwapChainFrame>& frames)
	{
		for (int i = 0; i < frames.size(); i++)
		{
			std::vector<vk::ImageView> attachments = { frames[i].imageView };

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = input.renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = input.swapchainextent.width;
			framebufferInfo.height = input.swapchainextent.height;
			framebufferInfo.layers = 1;

			try
			{
				frames[i].framebuffer = input.device.createFramebuffer(framebufferInfo);
				CONSOLE_INFO("Created framebuffer for frame %d.", i);
			}
			catch (vk::SystemError err)
			{
				CONSOLE_ERROR("Failed to Create Framebuffer for frame %d! %s", i, err.what());
			}
		}
	}
}

#endif