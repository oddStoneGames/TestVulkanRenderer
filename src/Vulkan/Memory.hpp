#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "../Config.hpp"

namespace vkInit
{
	struct BufferInput
	{
		std::size_t size;
		vk::BufferUsageFlags usage;
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
	};

	struct Buffer
	{
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;
	};

	inline uint32_t FindMemoryTypeIndex(const vk::PhysicalDevice& physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
	{
		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			bool supported = static_cast<bool>(supportedMemoryIndices & (1 << i));
			bool sufficient = (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

			if (supported && sufficient)
				return i;
		}

		return -1;
	}

	inline void AllocateBufferMemory(Buffer& buffer, const BufferInput& input)
	{
		vk::MemoryRequirements memoryRequirements = input.device.getBufferMemoryRequirements(buffer.buffer);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryTypeIndex(input.physicalDevice, memoryRequirements.memoryTypeBits, 
									vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		
		try
		{
			buffer.bufferMemory = input.device.allocateMemory(allocInfo);
			CONSOLE_INFO("Successfully allocated memory for buffer!");
		}
		catch (const vk::SystemError& err)
		{
			CONSOLE_INFO("Failed to allocate memory for buffer! %s", err.what());
			return;
		}
		
		input.device.bindBufferMemory(buffer.buffer, buffer.bufferMemory, 0);
	}

	inline Buffer CreateBuffer(const BufferInput& bufferInput)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.flags = vk::BufferCreateFlags();
		bufferInfo.size = bufferInput.size;
		bufferInfo.usage = bufferInput.usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		Buffer buffer;

		try
		{
			buffer.buffer = bufferInput.device.createBuffer(bufferInfo);
			CONSOLE_INFO("Successfully created Buffer!");
		}
		catch (const vk::SystemError& err)
		{
			CONSOLE_ERROR("Failed to create Buffer! %s", err.what());
			return buffer;
		}

		AllocateBufferMemory(buffer, bufferInput);

		return buffer;
	}
}

#endif