#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "../Config.hpp"
#include "../Shader.hpp"
#include "PushConstants.hpp"
#include "Mesh.hpp"

namespace vkInit
{
	struct GraphicsPipelineInBundle
	{
		vk::Device device;
		std::string vertexShaderFilePath;
		std::string fragmentShaderFilePath;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
	};

	struct GraphicsPipelineOutBundle
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};

	inline vk::PipelineLayout CreatePipelineLayout(const vk::Device& device)
	{
		vk::PipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pushConstantRangeCount = 1;
		vk::PushConstantRange pushConstantInfo{};
		pushConstantInfo.offset = 0;
		pushConstantInfo.size = sizeof(Constants);
		pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;
		layoutInfo.pPushConstantRanges = &pushConstantInfo;

		try
		{
			return device.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{
			CONSOLE_ERROR("Failed to Create Pipeline Layout! %s", err.what());
			return nullptr;
		}
	}

	vk::RenderPass CreateRenderPass(const vk::Device& device, vk::Format swapchainImageFormat)
	{
		vk::AttachmentDescription colorAttachment{};
		colorAttachment.flags = vk::AttachmentDescriptionFlags();
		colorAttachment.format = swapchainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass{};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::RenderPassCreateInfo renderpassInfo{};
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = 1;
		renderpassInfo.pAttachments = &colorAttachment;
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		try
		{
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err)
		{
			CONSOLE_ERROR("Failed to Create Render Pass! %s", err.what());
			return nullptr;
		}
	}

	inline GraphicsPipelineOutBundle MakeGraphicsPipeline(GraphicsPipelineInBundle specification)
	{
		vk::GraphicsPipelineCreateInfo createInfo{};
		createInfo.flags = vk::PipelineCreateFlags();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		// Vertex Input
		vk::VertexInputBindingDescription vertexBindingDescription = vkMesh::GetPosColorBindingDescription();
		std::array<vk::VertexInputAttributeDescription, 2> vertexAttributeDescriptions = vkMesh::GetPosColorAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = 2;
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
		createInfo.pVertexInputState = &vertexInputInfo;

		// Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		createInfo.pInputAssemblyState = &inputAssemblyInfo;

		// Vertex Shader
		vk::ShaderModule vertexShader = CreateModule(specification.vertexShaderFilePath, specification.device);
		vk::PipelineShaderStageCreateInfo vertexShaderInfo{};
		vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderInfo.module = vertexShader;
		vertexShaderInfo.pName = "main";
		shaderStages.push_back(vertexShaderInfo);

		// Viewport & Scissor
		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = specification.swapchainExtent.width;
		viewport.height = specification.swapchainExtent.height;

		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor{};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = specification.swapchainExtent;

		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		createInfo.pViewportState = &viewportState;

		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizerInfo{};
		rasterizerInfo.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizerInfo.depthClampEnable = VK_FALSE;
		rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizerInfo.lineWidth = 1.0f;
		rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
		rasterizerInfo.frontFace = vk::FrontFace::eClockwise;
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		createInfo.pRasterizationState = &rasterizerInfo;

		// Fragment Shader
		vk::ShaderModule fragmentShader = CreateModule(specification.fragmentShaderFilePath, specification.device);
		vk::PipelineShaderStageCreateInfo fragmentShaderInfo{};
		fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderInfo.module = fragmentShader;
		fragmentShaderInfo.pName = "main";
		shaderStages.push_back(fragmentShaderInfo);

		// Pass Shader Stages to Pipeline Create Info
		createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		createInfo.pStages = shaderStages.data();

		// Multisampling
		vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
		multisamplingInfo.flags = vk::PipelineMultisampleStateCreateFlags();
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		createInfo.pMultisampleState = &multisamplingInfo;

		// Color Blending
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
											  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		colorBlendAttachment.blendEnable = VK_FALSE;
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
		colorBlendInfo.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &colorBlendAttachment;
		colorBlendInfo.blendConstants[0] = 0.0f;
		colorBlendInfo.blendConstants[1] = 0.0f;
		colorBlendInfo.blendConstants[2] = 0.0f;
		colorBlendInfo.blendConstants[3] = 0.0f;
		createInfo.pColorBlendState = &colorBlendInfo;

		// Pipeline Layout
		vk::PipelineLayout pipelineLayout = CreatePipelineLayout(specification.device);
		createInfo.layout = pipelineLayout;

		// Renderpass
		vk::RenderPass renderPass = CreateRenderPass(specification.device, specification.swapchainImageFormat);
		createInfo.renderPass = renderPass;

		// Extra Stuff
		createInfo.basePipelineHandle = nullptr;
		
		// Create the Pipeline
		vk::Pipeline graphicsPipeline;
		GraphicsPipelineOutBundle output{};

		try
		{
			graphicsPipeline = (specification.device.createGraphicsPipeline(nullptr, createInfo)).value;
		}
		catch (vk::SystemError err)
		{
			CONSOLE_ERROR("Failed to Create Graphics Pipeline! %s", err.what());
			return output;
		}
		
		output.layout = pipelineLayout;
		output.renderpass = renderPass;
		output.pipeline = graphicsPipeline;

		specification.device.destroyShaderModule(vertexShader);
		specification.device.destroyShaderModule(fragmentShader);

		return output;
	}

}

#endif //!PIPELINE_HPP