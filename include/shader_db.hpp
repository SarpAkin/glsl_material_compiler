#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>

#include "file_header.hpp"

class ShaderDB {
public:
    void load_db(ShaderDBHeader* db_header) {
        size_t size = db_header->total_size;
        auto* copy  = reinterpret_cast<ShaderDBHeader*>(malloc(size));
        memcpy(copy, db_header, size);

        char* cursor = copy->data;

        for (int i = 0; i < copy->shader_count; ++i) {
            auto* pipeline = reinterpret_cast<CompiledPipeline*>(cursor);

            m_pipelinedbs[pipeline->shader_name] = pipeline;

            cursor += pipeline->total_size;
        }

        m_dbs.push_back(copy);
    }

    CompiledPipeline* get_pipeline_db(const char* name) {
        return m_pipelinedbs[name];
    }

    ~ShaderDB() {
        for (auto* db : m_dbs) {
            free(db);
        }
    }

private:
    std::unordered_map<std::string, CompiledPipeline*> m_pipelinedbs;
    std::vector<ShaderDBHeader*> m_dbs;
};