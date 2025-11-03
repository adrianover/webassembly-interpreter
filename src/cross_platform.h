#ifndef CROSS_PLATFORM_H
#define CROSS_PLATFORM_H

#pragma once
#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
inline int clz32(uint32_t x) {
    unsigned long index;
    _BitScanReverse(&index, x);
    return 31 - index;
}
inline int ctz32(uint32_t x) {
    unsigned long index;
    _BitScanForward(&index, x);
    return index;
}
inline int clz64(uint64_t x) {
    unsigned long index;
    _BitScanReverse64(&index, x);
    return 63 - index;
}
inline int ctz64(uint64_t x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return index;
}
#else
inline int clz32(uint32_t x) { return __builtin_clz(x); }
inline int ctz32(uint32_t x) { return __builtin_ctz(x); }
inline int clz64(uint64_t x) { return __builtin_clzll(x); }
inline int ctz64(uint64_t x) { return __builtin_ctzll(x); }
#endif

#endif //CROSS_PLATFORM_H
