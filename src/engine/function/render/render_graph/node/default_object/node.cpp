#include "./node.h"
#include "core/filesystem/file.h"
#include "core/vulkan/vulkan_util.h"
#include "function/global_context.h"
#include "function/render/render_graph/pipeline.hpp"
#include "function/resource_manager/resource_manager.h"

using namespace Vk;

DefaultObject::DefaultObject(const std::string& name, const std::string& color_buf_name, const std::string& depth_buf_name)
    : RenderGraphNode(name)
{
    assert(color_buf_name != RenderAttachmentDescription::SWAPCHAIN_IMAGE_NAME());
    attachment_descriptions = {
        {
            "color",
            {
                color_buf_name,
                RenderAttachmentType::Color,
                RenderAttachmentRW::Write,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_FORMAT_R32G32B32A32_SFLOAT,
            },
        },
        {
            "depth",
            RenderAttachmentDescription {
                depth_buf_name,
                RenderAttachmentType::Depth,
                RenderAttachmentRW::Read | RenderAttachmentRW::Write,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_FORMAT_D32_SFLOAT,
            },
        }
    };
}

void DefaultObject::init(Configuration& cfg, RenderAttachments& attachments)
{
    this->attachments = &attachments;
    createRenderPass();
    createFramebuffer();
    createPipeline(cfg);
}

void DefaultObject::createFramebuffer()
{
    framebuffers.resize(g_ctx.vk.swapChainImages.size());
    for (int i = 0; i < g_ctx.vk.swapChainImages.size(); i++) {
        std::array<VkImageView, 2> views = {
            attachments->getAttachment(attachment_descriptions["color"].name).view,
            attachments->getAttachment(attachment_descriptions["depth"].name).view,
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(views.size());
        framebufferInfo.pAttachments = views.data();
        framebufferInfo.width = g_ctx.vk.swapChainImages[i]->extent.width;
        framebufferInfo.height = g_ctx.vk.swapChainImages[i]->extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(g_ctx.vk.device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void DefaultObject::createRenderPass()
{
    std::vector<AttachmentDescriptionHelper> helpers = {
        { "color", VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE },
        { "depth", VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE },
    };
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    render_pass = DefaultRenderPass(attachment_descriptions, helpers, dependency);
}

void DefaultObject::createPipeline(Configuration& cfg)
{
    {
        std::vector<VkDescriptorSetLayout> descLayouts = {
            g_ctx.dm.BINDLESS_LAYOUT(),
            g_ctx.dm.PARAMETER_LAYOUT(),
            g_ctx.dm.PARAMETER_LAYOUT(),
        };
        pipeline.initLayout(descLayouts);
    }

    {
        VertexInputDefault(true);
        DynamicStateDefault();
        ViewportStateDefault();
        auto inputAssembly = Pipeline<Param>::inputAssemblyDefault();
        auto rasterization = Pipeline<Param>::rasterizationDefault();
        auto multisample = Pipeline<Param>::multisampleDefault();

        auto vertShaderCode = readFile(cfg.shader_directory + "/default_object/node.vert.spv");
        auto fragShaderCode = readFile(cfg.shader_directory + "/default_object/node.frag.spv");
        auto vertShaderModule = createShaderModule(g_ctx.vk, vertShaderCode);
        auto fragShaderModule = createShaderModule(g_ctx.vk, fragShaderCode);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            Pipeline<Param>::shaderStageDefault(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT),
            Pipeline<Param>::shaderStageDefault(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT),
        };
        VkPipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
        VkPipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        VkPipelineDepthStencilStateCreateInfo depthStencil {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterization;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipeline.layout;
        pipelineInfo.renderPass = render_pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional
        if (vkCreateGraphicsPipelines(g_ctx.vk.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        vkDestroyShaderModule(g_ctx.vk.device, vertShaderModule, nullptr);
        vkDestroyShaderModule(g_ctx.vk.device, fragShaderModule, nullptr);
    }

    {
        pipeline.param.camera = g_ctx.dm.getResourceHandle(g_ctx.rm->camera.buffer.id);
        pipeline.param.lights = g_ctx.dm.getResourceHandle(g_ctx.rm->lights.buffer.id);
        pipeline.param_buf = Buffer::New(
            g_ctx.vk,
            sizeof(Param),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true);
        pipeline.param_buf.Update(g_ctx.vk, &pipeline.param, sizeof(Param));
        g_ctx.dm.registerParameter(pipeline.param_buf);
    }
}

void DefaultObject::record(uint32_t swapchain_index)
{
    setDefaultViewportAndScissor();

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } }; // dummy
    clearValues[1].depthStencil = { 1.0f, 0 };
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = framebuffers[swapchain_index];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = toVkExtent2D(g_ctx.vk.swapChainImages[swapchain_index]->extent);
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(g_ctx.vk.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(g_ctx.vk.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    bindDescriptorSet(0, pipeline.layout, g_ctx.dm.BINDLESS_SET());
    bindDescriptorSet(1, pipeline.layout, g_ctx.dm.getParameterSet(pipeline.param_buf.id));

    for (const auto& obj : g_ctx.rm->objects) {
        bindDescriptorSet(2, pipeline.layout, g_ctx.dm.getParameterSet(obj.paramBuffer.id));
        const auto& mesh = g_ctx.rm->meshes[obj.mesh];

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(g_ctx.vk.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(g_ctx.vk.commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(g_ctx.vk.commandBuffer, mesh.data.indices.size(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(g_ctx.vk.commandBuffer);
}

void DefaultObject::onResize()
{
    for (auto& framebuffer : framebuffers) {
        vkDestroyFramebuffer(g_ctx.vk.device, framebuffer, nullptr);
    }
    createFramebuffer();
}

void DefaultObject::destroy()
{
    pipeline.destroy();
    vkDestroyRenderPass(g_ctx.vk.device, render_pass, nullptr);
    for (auto& framebuffer : framebuffers) {
        vkDestroyFramebuffer(g_ctx.vk.device, framebuffer, nullptr);
    }
}
