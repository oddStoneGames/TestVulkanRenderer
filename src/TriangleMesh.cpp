#include "TriangleMesh.hpp"

TriangleMesh::TriangleMesh(const vk::Device& device, const vk::PhysicalDevice& physicalDevice)
{
	m_LogicalDevice = device;

	std::vector<float> vertices =
	{
		0.0f, -0.05f, 1.0f, 0.0f, 0.0f,
		0.05f, 0.05f, 0.0f, 1.0f, 0.0f,
		-0.05f, 0.05f, 0.0f, 0.0f, 1.0f
	};

	vkInit::BufferInput inputChunk;
	inputChunk.device = device;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.size = sizeof(float) * vertices.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer;

	vertexBuffer = vkInit::CreateBuffer(inputChunk);

	void* memoryLocation = device.mapMemory(vertexBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertices.data(), inputChunk.size);
	device.unmapMemory(vertexBuffer.bufferMemory);
}

void TriangleMesh::Destroy()
{
	m_LogicalDevice.waitIdle();

	m_LogicalDevice.destroyBuffer(vertexBuffer.buffer);
	m_LogicalDevice.freeMemory(vertexBuffer.bufferMemory);
}