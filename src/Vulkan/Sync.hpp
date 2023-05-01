#ifndef SYNC_HPP
#define SYNC_HPP

#include "../Config.hpp"

namespace vkInit
{
	vk::Semaphore CreateSemaphore(const vk::Device& device)
	{
		vk::SemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.flags = vk::SemaphoreCreateFlags();

		try
		{
			return device.createSemaphore(semaphoreInfo);
		}
		catch (const vk::SystemError& err)
		{
			CONSOLE_ERROR("Failed to Create Semaphore! %s", err.what());
			return nullptr;
		}
	}

	vk::Fence CreateFence(const vk::Device& device)
	{
		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try
		{
			return device.createFence(fenceInfo);
		}
		catch (const vk::SystemError& err)
		{
			CONSOLE_ERROR("Failed to Create Fence! %s", err.what());
			return nullptr;
		}
	}
}

#endif