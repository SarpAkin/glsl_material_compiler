#pragma once

#include <vulkan/vulkan.h>

#include <span>

struct CompiledSpv {
    VkShaderStageFlagBits stage;
    uint32_t offset_in_bytes; // Relative to the compiled shader
    uint32_t size_in_bytes;
};

struct CompiledPipeline {
    uint32_t total_size;
    char shader_name[44];
    char renderpass_name[32];
    char vertex_input_name[32];
    VkPolygonMode polygon_mode;
    VkCullModeFlags cull_mode;
    VkPrimitiveTopology topology;
    VkCompareOp depth_op;
    bool depth_test;
    bool depth_write;
    uint8_t stage_count;
    CompiledSpv stages[5]; //

    char data[];

    std::span<const uint32_t> get_stage_spv(int stage_index) const {
        const CompiledSpv& stage = stages[stage_index];

        return std::span(reinterpret_cast<const uint32_t*>(&data[0] + stage.offset_in_bytes), stage.size_in_bytes / 4);
    }
};

struct ShaderDBHeader {
    uint32_t total_size;
    uint32_t shader_count;

    char data[];
};
