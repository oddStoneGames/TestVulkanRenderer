#ifndef TRIANGLE_MESH_HPP
#define TRIANGLE_MESH_HPP

#include "Config.hpp"
#include "Vulkan/Memory.hpp"

class TriangleMesh
{
public:
	TriangleMesh(const vk::Device& device, const vk::PhysicalDevice& physicalDevice);
	~TriangleMesh() {}
	void Destroy();
	vkInit::Buffer vertexBuffer;
private:
	vk::Device m_LogicalDevice;
};

#endif // !TRIANGLE_MESH_HPP