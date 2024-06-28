#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::vector<uint32_t> compile_glsl(const std::string& path, const std::vector<std::pair<std::string, std::string>>& flags);