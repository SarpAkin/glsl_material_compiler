#pragma once

#include <cstdint>
#include <cstring>
#include <span>

namespace vke {

using usize = size_t;
using u8    = uint8_t;

class ArenaAllocator {
public:
    ArenaAllocator();
    ~ArenaAllocator();

    void* alloc(usize size);
    void reset() { m_top = m_base; }

    template <typename T>
    T* alloc(usize count = 1) { return reinterpret_cast<T*>(alloc(count * sizeof(T))); }

    template <typename T>
    T* calloc(usize count = 1) {
        auto* data = alloc(count * sizeof(T));
        memset(data, 0, count * sizeof(T));
        return reinterpret_cast<T*>(data);
    }


    template <typename T>
    std::span<T> create_copy(std::span<const T> src) {
        T* dst = alloc<T>(src.size());
        memcpy(dst, src.data(), src.size_bytes());
        return std::span<T>(dst, src.size());
    }

    template <typename T>
    std::span<T> create_copy(std::span<T> src) {
        T* dst = alloc<T>(src.size());
        memcpy(dst, src.data(), src.size_bytes());
        return std::span<T>(dst, src.size());
    }

    template <typename T>
    T* create_copy(const T& src) {
        T* dst = alloc<T>(1);
        memcpy(dst, &src, sizeof(T));
        return dst;
    }

    const char* create_str_copy(const char* str, usize* out_len = nullptr);

    ArenaAllocator(const ArenaAllocator&)            = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

private:
    u8* m_base;
    u8* m_top;
    u8* m_cap;
};

class ArenaGen {
};

} // namespace vke