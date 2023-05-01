#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../Config.hpp"
#include "QueueFamily.hpp"

namespace vkInit
{
	struct CommandBufferInputChunk
	{
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<SwapChainFrame>& frames;
	};

	vk::CommandPool CreateCommandPool(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		try
		{
			return device.createCommandPool(poolInfo);
		}
		catch (vk::SystemError err)
		{
			CONSOLE_ERROR("Failed to Create Command Pool! %s", err.what());
			return nullptr;
		}
	}

	vk::CommandBuffer CreateCommandBuffers(CommandBufferInputChunk input)
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = input.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		for (int i = 0; i < input.frames.size(); i++)
		{
			try
			{
				input.frames[i].commandBuffer = input.device.allocateCommandBuffers(allocInfo)[0];
				CONSOLE_INFO("Allocated command buffer for frame %d", i);

			}
			catch (const vk::SystemError& err)
			{
				CONSOLE_ERROR("Failed to Allocate command buffer for frame %d! %s", i, err.what());
			}
		}

		try
		{
			vk::CommandBuffer commandBuffer = input.device.allocateCommandBuffers(allocInfo)[0];
			CONSOLE_INFO("Allocated command buffer for engine.");
			return commandBuffer;
		}
		catch (const vk::SystemError& err)
		{
			CONSOLE_ERROR("Failed to Allocate command buffer for engine! %s", err.what());
			return nullptr;
		}
	}
}

#endif