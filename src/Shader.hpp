#ifndef SHADER_HPP
#define SHADER_HPP

#include "Config.hpp"
#include <fstream>

inline std::vector<char> ReadFile(std::string filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		CONSOLE_ERROR("Failed to open %s", filePath);

	std::size_t fileSize{ static_cast<size_t>(file.tellg()) };

	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

inline vk::ShaderModule CreateModule(std::string filePath, const vk::Device& device)
{
	std::vector<char> sourceCode = ReadFile(filePath);
	vk::ShaderModuleCreateInfo moduleInfo{};
	moduleInfo.flags = vk::ShaderModuleCreateFlags();
	moduleInfo.codeSize = sourceCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

	try 
	{
		return device.createShaderModule(moduleInfo);
		CONSOLE_INFO("Successfully created ShaderModule for %s.", filePath);
	}
	catch (vk::SystemError err)
	{
		CONSOLE_ERROR("Failed to Create ShaderModule for %s: %s", filePath, err.what());
		return nullptr;
	}
}

#endif // !SHADER_HPP