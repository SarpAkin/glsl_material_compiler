#pragma once

#include <string>
#include <vulkan/vulkan.h>

VkPolygonMode parse_polygon_mode(const std::string& str);

VkPrimitiveTopology parse_topology_mode(const std::string& str);

VkCullModeFlags parse_cull_mode(const std::string& str);

VkCompareOp parse_depth_op(const std::string& str);

VkShaderStageFlagBits infer_shader_stage(const std::string& filepath);
