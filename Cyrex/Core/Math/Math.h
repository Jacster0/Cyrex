#pragma once
namespace Cyrex::Math {
    template<typename T>
    inline T AlignUpWithMask(T value, size_t mask) {
        return static_cast<T>((static_cast<size_t>(value + mask) & ~mask));
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask) {
        return static_cast<T>((static_cast<size_t>(value) & ~mask));
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment) {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment) {
        return AlignDownWithMask(value, alignment - 1);
    }

    template<typename T>
    inline bool IsAligned(T value, size_t alignment) {
        return 0 == (static_cast<size_t>(value) & (alignment - 1));
    }
}