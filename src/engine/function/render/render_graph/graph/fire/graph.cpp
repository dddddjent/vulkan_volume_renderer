#include "graph.h"

void FireGraph::init(Configuration& cfg)
{
    nodes["FireObject"]
        = std::move(std::make_unique<FireObject>("FireObject", "object_color", "depth"));
    nodes["Field"]
        = std::move(std::make_unique<FieldNode>("Field", "object_color", "depth", "field_object_color"));
    nodes["HDRToSDR"]
        = std::move(std::make_unique<HDRToSDR>("HDRToSDR", "field_object_color", RenderAttachmentDescription::SWAPCHAIN_IMAGE_NAME()));
    nodes["UI"]
        = std::move(std::make_unique<UI>("UI", RenderAttachmentDescription::SWAPCHAIN_IMAGE_NAME(), fn));
    nodes["Record"]
        = std::move(std::make_unique<Record>("Record", RenderAttachmentDescription::SWAPCHAIN_IMAGE_NAME()));
    initAttachments();

    for (auto& node : nodes) {
        node.second->init(cfg, attachments);
    }

    graph = {
        { "Field", { "FireObject" } },
        { "HDRToSDR", { "Field" } },
        { "Record", { "HDRToSDR" } },
        { "UI", { "Record", "HDRToSDR" } },
    };
    initGraph();
}
