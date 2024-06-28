#pragma once

#include <string_view>

namespace vke {
class ArenaAllocator;
}

std::string_view read_file(vke::ArenaAllocator* arena, const char* name);