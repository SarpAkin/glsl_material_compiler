#include "shader_compiler.hpp"

#include <filesystem>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <string>

#include <arena_alloc.hpp>

using vke::ArenaAllocator;
namespace fs = std::filesystem;

std::string_view read_file(vke::ArenaAllocator* arena, const char* name) {
    std::ifstream file(name, std::ios::ate);
    if (!file.is_open()) {
        char error[120];

        snprintf(error, sizeof(error), "failed to open file: %s", name);

        throw std::runtime_error("failed to open file");
    }
    size_t filesize = static_cast<size_t>(file.tellg());
    char* data      = arena->alloc<char>(filesize + 1);
    data[filesize]  = '\0';

    file.seekg(0);
    file.read(reinterpret_cast<char*>(data), filesize);
    return std::string_view(data, filesize);
}

class ShadercIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    ShadercIncluder(ArenaAllocator* _arena) {
        m_arena = _arena;
    }

    ArenaAllocator* m_arena;

    shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override {
        if (type == shaderc_include_type_standard) return nullptr;

        fs::path path = fs::path(requesting_source).parent_path() / requested_source;

        auto content = read_file(m_arena, path.c_str());

        size_t pathc_len;
        const char* pathc = m_arena->create_str_copy(path.c_str(), &pathc_len);

        return m_arena->create_copy(shaderc_include_result{
            .source_name        = pathc,
            .source_name_length = pathc_len,
            .content            = content.begin(),
            .content_length     = content.size(),
        });
    }

    void ReleaseInclude(shaderc_include_result* data) override {
    }
};

// Function to compile GLSL shader
shaderc::SpvCompilationResult compileGLSL(const std::string& file_path, shaderc_shader_kind kind = shaderc_glsl_infer_from_source) {
    ArenaAllocator arena;

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetSpirv(shaderc_spirv_version_1_5);

    // options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetIncluder(std::make_unique<ShadercIncluder>(&arena));

    auto source = read_file(&arena, file_path.c_str());

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.begin(), source.size(), kind, file_path.c_str(), "main", options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error("Shader compilation failed: " + std::string(result.GetErrorMessage()));
    }

    return result;
}

shaderc_shader_kind inferShaderType(const std::string& filePath) {
    if (filePath.ends_with(".frag") || filePath.ends_with(".fsh")) return shaderc_glsl_fragment_shader;
    if (filePath.ends_with(".vert") || filePath.ends_with(".vsh")) return shaderc_glsl_vertex_shader;
    if (filePath.ends_with(".geom")) return shaderc_glsl_geometry_shader;
    if (filePath.ends_with(".comp")) return shaderc_glsl_compute_shader;
    if (filePath.ends_with(".tesc")) return shaderc_glsl_tess_control_shader;
    if (filePath.ends_with(".tese")) return shaderc_glsl_tess_evaluation_shader;
    if (filePath.ends_with(".mesh")) return shaderc_glsl_mesh_shader;
    if (filePath.ends_with(".task")) return shaderc_glsl_task_shader;

    throw std::runtime_error("Unknown shader file extension: " + filePath);
}

std::vector<uint32_t> compile_glsl(const std::string& file_path, const std::vector<std::pair<std::string, std::string>>& flags) {
    ArenaAllocator arena;

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetSpirv(shaderc_spirv_version_1_5);

    // options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetIncluder(std::make_unique<ShadercIncluder>(&arena));

    auto source = read_file(&arena, file_path.c_str());

    for(auto& [name,definition] : flags){
        options.AddMacroDefinition(name,definition);
    }

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.begin(), source.size(), inferShaderType(file_path), file_path.c_str(), "main", options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error("Shader compilation failed: " + std::string(result.GetErrorMessage()));
    }

    return std::vector<uint32_t>(result.begin(), result.end());
}