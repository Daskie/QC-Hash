#pragma once

#include <cstdint>

#include <utility>

//
// Murmur3 - https://github.com/PeterScott/murmur3
//
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// Note - The x86 and x64 versions do not produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with
// the non-native version will be less than optimal.
//
namespace qc_hash::murmur3 {

    //
    // 32 bit perfect integer hash.
    //
    constexpr uint32_t mix32(uint32_t h) noexcept;

    //
    // 64 bit perfect integer hash.
    //
    constexpr uint64_t mix64(uint64_t h) noexcept;

    //
    // Produces a 32 bit hash; optimized for x86 platforms.
    //
    uint32_t x86_32(const void * key, uint32_t n, uint32_t seed) noexcept;

    //
    // Produces a 128 bit hash; optimized for x86 platforms.
    //
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> x86_128(const void * key, uint32_t n, uint32_t seed) noexcept;

    //
    // Produces a 128 bit hash; optimized for x64 platforms.
    //
    std::pair<uint64_t, uint64_t> x64_128(const void * key, uint64_t n, uint64_t seed) noexcept;

    //
    // Wrapper that elects the best murmur3 function based on the current architecture.
    //
    size_t hash(const void * key, size_t n, size_t seed = 0u) noexcept;

}

namespace qc_hash::murmur3 {

    inline constexpr uint32_t _rotl32(const uint32_t x, const int r) noexcept {
        return (x << r) | (x >> (32 - r));
    }

    inline constexpr uint64_t _rotl64(const uint64_t x, const int r) noexcept {
        return (x << r) | (x >> (64 - r));
    }

    inline constexpr uint32_t mix32(uint32_t h) noexcept {
        h ^= h >> 16;
        h *= 0x85EBCA6Bu;
        h ^= h >> 13;
        h *= 0xC2B2AE35u;
        h ^= h >> 16;

        return h;
    }

    inline constexpr uint64_t mix64(uint64_t h) noexcept {
        h ^= h >> 33;
        h *= 0xFF51AFD7ED558CCDu;
        h ^= h >> 33;
        h *= 0xC4CEB9FE1A85EC53u;
        h ^= h >> 33;

        return h;
    }

    inline uint32_t x86_32(const void * const key, const uint32_t n, const uint32_t seed) noexcept {
        static constexpr uint32_t c1{0xCC9E2D51u};
        static constexpr uint32_t c2{0x1B873593u};

        const uint8_t * const data{reinterpret_cast<const uint8_t *>(key)};
        const uint32_t nblocks{n >> 2};
        const uint32_t nbytes{nblocks << 2};
        const uint32_t * const blocks{reinterpret_cast<const uint32_t *>(data + nbytes)};
        const uint8_t * const tail{reinterpret_cast<const uint8_t *>(data + nbytes)};

        uint32_t h1{seed};

        for (int32_t i{-int32_t(nblocks)}; i < 0; ++i) {
            uint32_t k1{blocks[i]};

            k1 *= c1;
            k1  = _rotl32(k1, 15);
            k1 *= c2;

            h1 ^= k1;
            h1  = _rotl32(h1, 13);
            h1  = h1 * 5u + 0xE6546B64u;
        }

        uint32_t k1{0u};

        switch (n & 0b11u) {
            case 0b11u: k1 ^= uint32_t(tail[2]) << 16; [[fallthrough]];
            case 0b10u: k1 ^= uint32_t(tail[1]) <<  8; [[fallthrough]];
            case 0b01u: k1 ^= uint32_t(tail[0]);
                k1 *= c1;
                k1  = _rotl32(k1, 15);
                k1 *= c2;
                h1 ^= k1;
        }

        h1 ^= n;

        h1 = mix32(h1);

        return h1;
    }

    inline std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> x86_128(const void * const key, const uint32_t n, const uint32_t seed) noexcept {
        static constexpr uint32_t c1{0x239B961Bu};
        static constexpr uint32_t c2{0xAB0E9789u};
        static constexpr uint32_t c3{0x38B34AE5u};
        static constexpr uint32_t c4{0xA1E38B93u};

        const uint8_t * const data{reinterpret_cast<const uint8_t *>(key)};
        const int32_t nblocks{int32_t(n >> 4)};
        const int32_t nbytes{nblocks << 4};
        const uint32_t * const blocks{reinterpret_cast<const uint32_t *>(data + nbytes)};
        const uint8_t * const tail{data + nbytes};

        uint32_t h1{seed};
        uint32_t h2{seed};
        uint32_t h3{seed};
        uint32_t h4{seed};

        for (int32_t i{-nblocks}; i < 0; ++i) {
            const int32_t i4{i << 2};
            uint32_t k1{blocks[i4 + 0u]};
            uint32_t k2{blocks[i4 + 1u]};
            uint32_t k3{blocks[i4 + 2u]};
            uint32_t k4{blocks[i4 + 3u]};

            k1 *= c1;
            k1  = _rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;

            h1  = _rotl32(h1, 19);
            h1 += h2;
            h1  = h1 * 5u + 0x561CCD1Bu;

            k2 *= c2;
            k2  = _rotl32(k2, 16);
            k2 *= c3;
            h2 ^= k2;

            h2  = _rotl32(h2, 17);
            h2 += h3;
            h2  = h2 * 5u + 0x0BCAA747u;

            k3 *= c3;
            k3  = _rotl32(k3, 17);
            k3 *= c4;
            h3 ^= k3;

            h3  = _rotl32(h3, 15);
            h3 += h4;
            h3  = h3 * 5u + 0x96CD1C35u;

            k4 *= c4;
            k4  = _rotl32(k4, 18);
            k4 *= c1;
            h4 ^= k4;

            h4  = _rotl32(h4, 13);
            h4 += h1;
            h4  = h4 * 5u + 0x32AC3B17u;
        }

        uint32_t k1{0u};
        uint32_t k2{0u};
        uint32_t k3{0u};
        uint32_t k4{0u};

        switch (n & 0b1111u) {
            case 15u: k4 ^= uint32_t(tail[14]) << 16; [[fallthrough]];
            case 14u: k4 ^= uint32_t(tail[13]) <<  8; [[fallthrough]];
            case 13u: k4 ^= uint32_t(tail[12]) <<  0;
                k4 *= c4;
                k4  = _rotl32(k4, 18);
                k4 *= c1;
                h4 ^= k4;
                [[fallthrough]];

            case 12u: k3 ^= uint32_t(tail[11]) << 24; [[fallthrough]];
            case 11u: k3 ^= uint32_t(tail[10]) << 16; [[fallthrough]];
            case 10u: k3 ^= uint32_t(tail[ 9]) <<  8; [[fallthrough]];
            case  9u: k3 ^= uint32_t(tail[ 8]) <<  0;
                k3 *= c3;
                k3  = _rotl32(k3, 17);
                k3 *= c4;
                h3 ^= k3;
                [[fallthrough]];

            case 8u: k2 ^= uint32_t(tail[7]) << 24; [[fallthrough]];
            case 7u: k2 ^= uint32_t(tail[6]) << 16; [[fallthrough]];
            case 6u: k2 ^= uint32_t(tail[5]) <<  8; [[fallthrough]];
            case 5u: k2 ^= uint32_t(tail[4]) <<  0;
                k2 *= c2;
                k2  = _rotl32(k2, 16);
                k2 *= c3;
                h2 ^= k2;
                [[fallthrough]];

            case 4u: k1 ^= uint32_t(tail[3]) << 24; [[fallthrough]];
            case 3u: k1 ^= uint32_t(tail[2]) << 16; [[fallthrough]];
            case 2u: k1 ^= uint32_t(tail[1]) <<  8; [[fallthrough]];
            case 1u: k1 ^= uint32_t(tail[0]) <<  0;
                k1 *= c1;
                k1  = _rotl32(k1, 15);
                k1 *= c2;
                h1 ^= k1;
        }

        h1 ^= n;
        h2 ^= n;
        h3 ^= n;
        h4 ^= n;

        h1 += h2;
        h1 += h3;
        h1 += h4;
        h2 += h1;
        h3 += h1;
        h4 += h1;

        h1 = mix32(h1);
        h2 = mix32(h2);
        h3 = mix32(h3);
        h4 = mix32(h4);

        h1 += h2;
        h1 += h3;
        h1 += h4;
        h2 += h1;
        h3 += h1;
        h4 += h1;

        return {h1, h2, h3, h4};
    }

    inline std::pair<uint64_t, uint64_t> x64_128(const void * const key, const uint64_t n, const uint64_t seed) noexcept {
        static constexpr uint64_t c1{0x87C37B91114253D5u};
        static constexpr uint64_t c2{0x4CF5AD432745937Fu};

        const uint8_t * const data{reinterpret_cast<const uint8_t *>(key)};
        const uint64_t nblocks{n >> 4};
        const uint64_t nbytes{nblocks << 4};
        const uint64_t * const blocks{reinterpret_cast<const uint64_t *>(data)};
        const uint8_t * const tail{data + nbytes};

        uint64_t h1{seed};
        uint64_t h2{seed};

        for (uint64_t i{0u}; i < nblocks; ++i) {
            const uint64_t i2{i << 1};
            uint64_t k1{blocks[i2 + 0u]};
            uint64_t k2{blocks[i2 + 1u]};

            k1 *= c1;
            k1  = _rotl64(k1, 31);
            k1 *= c2;
            h1 ^= k1;

            h1  = _rotl64(h1, 27);
            h1 += h2;
            h1  = h1 * 5u + 0x52DCE729u;

            k2 *= c2;
            k2  = _rotl64(k2, 33);
            k2 *= c1;
            h2 ^= k2;

            h2  = _rotl64(h2, 31);
            h2 += h1;
            h2  = h2 * 5u + 0x38495AB5u;
        }

        uint64_t k1{0u};
        uint64_t k2{0u};

        switch (n & 0b1111u) {
            case 15u: k2 ^= uint64_t(tail[14]) << 48; [[fallthrough]];
            case 14u: k2 ^= uint64_t(tail[13]) << 40; [[fallthrough]];
            case 13u: k2 ^= uint64_t(tail[12]) << 32; [[fallthrough]];
            case 12u: k2 ^= uint64_t(tail[11]) << 24; [[fallthrough]];
            case 11u: k2 ^= uint64_t(tail[10]) << 16; [[fallthrough]];
            case 10u: k2 ^= uint64_t(tail[ 9]) <<  8; [[fallthrough]];
            case  9u: k2 ^= uint64_t(tail[ 8]) <<  0;
                k2 *= c2;
                k2  = _rotl64(k2, 33);
                k2 *= c1;
                h2 ^= k2;
                [[fallthrough]];

            case 8u: k1 ^= uint64_t(tail[7]) << 56; [[fallthrough]];
            case 7u: k1 ^= uint64_t(tail[6]) << 48; [[fallthrough]];
            case 6u: k1 ^= uint64_t(tail[5]) << 40; [[fallthrough]];
            case 5u: k1 ^= uint64_t(tail[4]) << 32; [[fallthrough]];
            case 4u: k1 ^= uint64_t(tail[3]) << 24; [[fallthrough]];
            case 3u: k1 ^= uint64_t(tail[2]) << 16; [[fallthrough]];
            case 2u: k1 ^= uint64_t(tail[1]) <<  8; [[fallthrough]];
            case 1u: k1 ^= uint64_t(tail[0]) <<  0;
                k1 *= c1;
                k1  = _rotl64(k1, 31);
                k1 *= c2;
                h1 ^= k1;
        }

        h1 ^= n;
        h2 ^= n;

        h1 += h2;
        h2 += h1;

        h1 = mix64(h1);
        h2 = mix64(h2);

        h1 += h2;
        h2 += h1;

        return {h1, h2};
    }

    inline size_t hash(const void * const key, const size_t n, const size_t seed) noexcept {
        if constexpr (sizeof(size_t) == 4u) {
            return x86_32(key, n, seed);
        }
        else if constexpr (sizeof(size_t) == 8u) {
            return x64_128(key, n, seed).first;
        }
    }

}