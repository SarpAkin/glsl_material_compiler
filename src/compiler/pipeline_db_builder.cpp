#include "pipeline_db_builder.hpp"

#include <filesystem>
#include <fstream>

#include "shader_compiler.hpp"
#include "vk_utlls.hpp"

void if_exist(nh::json::value_type& root, const char* field, auto&& func) {
    if (root.contains(field)) func(root.at(field));
}

void try_to_get_field_into_char_arr(nh::json::value_type& root, const char* field, std::span<char> cstr) {
    if_exist(root, field, [&](nh::json::value_type& val) {
        auto str_val = val.get<std::string>();
        memcpy(cstr.data(), str_val.c_str(), std::min(cstr.size(), str_val.size()));
    });
}

void set_default_values(CompiledPipeline* pipelinedb) {
    pipelinedb->total_size = sizeof(CompiledPipeline);
    strcpy(pipelinedb->shader_name, "null");
    strcpy(pipelinedb->renderpass_name, "null");
    strcpy(pipelinedb->vertex_input_name, "null");
    pipelinedb->polygon_mode = VK_POLYGON_MODE_FILL;
    pipelinedb->cull_mode    = VK_CULL_MODE_NONE;
    pipelinedb->topology     = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelinedb->depth_op     = VK_COMPARE_OP_LESS;
    pipelinedb->depth_test   = false;
    pipelinedb->depth_write  = false;
    pipelinedb->stage_count  = 0;
    memset(pipelinedb->stages, 0, sizeof(pipelinedb->stages));
}

bool PipelineDBConstructor::compile_material_file(const char* file_name) {
    fs::path path = file_name;
    path          = path.parent_path();

    m_scratch.reset();

    auto fdata = read_file(&m_scratch, file_name);
    auto json  = nh::json::parse(fdata);

    auto arr = json["pipelines"];
    if (!arr.is_array()) return false;

    for (auto& val : arr) {

        auto* pipelinedb = compile_pipeline(val, path);
        if (pipelinedb)
            m_pipelinedbs.push_back(pipelinedb);
    }

    return true;
}

CompiledPipeline* PipelineDBConstructor::compile_pipeline(nh::json::value_type& val, fs::path material_dir) {
    auto* pipelinedb = m_data.calloc<CompiledPipeline>();

    try {
        set_default_values(pipelinedb);

        try_to_get_field_into_char_arr(val, "name", pipelinedb->shader_name);
        try_to_get_field_into_char_arr(val, "renderpass", pipelinedb->renderpass_name);
        try_to_get_field_into_char_arr(val, "vertex_input", pipelinedb->vertex_input_name);

        if_exist(val, "depth_test", [&](nh::json::value_type& val) { val.get_to(pipelinedb->depth_test); });
        if_exist(val, "depth_write", [&](nh::json::value_type& val) { val.get_to(pipelinedb->depth_write); });

        if_exist(val, "polygon_mode", [&](nh::json::value_type& val) {
            pipelinedb->polygon_mode = parse_polygon_mode(val.get<std::string>());
        });
        if_exist(val, "topology_mode", [&](nh::json::value_type& val) {
            pipelinedb->topology = parse_topology_mode(val.get<std::string>());
        });
        if_exist(val, "cull_mode", [&](nh::json::value_type& val) {
            pipelinedb->cull_mode = parse_cull_mode(val.get<std::string>());
        });
        if_exist(val, "depth_op", [&](nh::json::value_type& val) {
            pipelinedb->depth_op = parse_depth_op(val.get<std::string>());
        });

        std::vector<std::pair<std::string, std::string>> definitions;
        if_exist(val, "compiler_definitions", [&](nh::json::value_type& defs) {
            for (auto it = defs.begin(); it != defs.end(); ++it) {
                definitions.emplace_back(it.key(), it.value().get<std::string>());
            }
        });

        if_exist(val, "shader_files", [&](nh::json::value_type& val) {
            if (val.is_array()) {
                for (auto& shader_file : val) {
                    std::string shader_path = material_dir / shader_file.get<std::string>();
                    if (!compile_stage(pipelinedb, shader_path, definitions)) {
                        // TODO Handle compilation failure
                    }
                }
            }
        });
    } catch (const std::exception& e) {
        fprintf(stderr, "error while compiling pipeline: %s\n", e.what());
        return nullptr;
    }

    return pipelinedb;
}

bool PipelineDBConstructor::compile_stage(CompiledPipeline* pipelinedb, const std::string& shader_filename, std::vector<std::pair<std::string, std::string>> definitions) {
    std::vector<uint32_t> compiled_code = compile_glsl(shader_filename, definitions);

    if (compiled_code.empty()) {
        return false;
    }

    // Assuming stage_count corresponds to the index of the next stage to be filled
    if (pipelinedb->stage_count >= 5) {
        return false; // Maximum stages reached
    }

    CompiledSpv& stage    = pipelinedb->stages[pipelinedb->stage_count];
    stage.size_in_bytes   = compiled_code.size() * sizeof(uint32_t);
    stage.offset_in_bytes = pipelinedb->total_size - sizeof(CompiledPipeline); // total size includes the header
    stage.stage           = infer_shader_stage(shader_filename);

    auto* spv_data = reinterpret_cast<char*>(m_data.alloc(stage.size_in_bytes));

    // no other allocations have happened
    assert(spv_data == &pipelinedb->data[stage.offset_in_bytes]);

    // Increment total_size by the size of the new stage
    pipelinedb->total_size += stage.size_in_bytes;

    // Copy the compiled code to the data array
    memcpy(&pipelinedb->data[stage.offset_in_bytes], compiled_code.data(), stage.size_in_bytes);

    // Increment the stage count
    pipelinedb->stage_count++;

    return true;
}

bool PipelineDBConstructor::dump_to_file(const char* file_name) {
    std::ofstream file(file_name, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    ShaderDBHeader header{
        .shader_count = static_cast<uint32_t>(m_pipelinedbs.size()),
    };

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Iterate over all compiled pipelines
    for (auto* pipelinedb : m_pipelinedbs) {
        // Write the CompiledPipeline structure itself
        file.write(reinterpret_cast<const char*>(pipelinedb), pipelinedb->total_size);
    }

    header.total_size = file.tellp();

    file.seekp(offsetof(ShaderDBHeader, total_size));
    file.write(reinterpret_cast<const char*>(&header.total_size), sizeof(header.total_size));

    file.close();
    return true;
}