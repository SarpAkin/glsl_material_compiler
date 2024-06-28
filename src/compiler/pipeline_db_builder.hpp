#pragma once

#include <json.hpp>
#include <string>
#include <vulkan/vulkan.h>
#include <filesystem>


#include <arena_alloc.hpp>
#include <file_header.hpp>

#include "util.hpp"

namespace nh = nlohmann;
namespace fs = std::filesystem;

class PipelineDBConstructor {
public:
    bool compile_material_file(const char* file_name);
    bool dump_to_file(const char* file_name);

private:
    bool compile_stage(CompiledPipeline* pipelinedb, const std::string& shader_filename, std::vector<std::pair<std::string, std::string>> definitions);
    CompiledPipeline* compile_pipeline(nh::json::value_type& root_node, fs::path material_dir);

private:
    vke::ArenaAllocator m_scratch;
    vke::ArenaAllocator m_data;
    std::vector<CompiledPipeline*> m_pipelinedbs;
};