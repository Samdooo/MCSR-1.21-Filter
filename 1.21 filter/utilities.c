#include <math.h>

#include "cubiomes/rng.h"
#include "utilities.h"

#define INT_MAX 2147483647

int clamp(const int v, const int mn, const int mx) {
    if (v < mn) return mn;
    if (v > mx) return mx;
    return v;
}

int max(int x, int y) {
    if (x >= y) {
        return x;
    }
    return y;
}

int min(int x, int y) {
    if (x <= y) {
        return x;
    }
    return y;
}

uint64_t get_population_seed(const uint64_t world_seed, const int block_x, const int block_z) {
    Xoroshiro xr;
    xSetSeed(&xr, world_seed);
    uint64_t a = xNextLongJ(&xr) | 1;
    uint64_t b = xNextLongJ(&xr) | 1;
    
    uint64_t population_seed = ((uint64_t)block_x * a + (uint64_t)block_z * b) ^ world_seed;
    return population_seed;
}

// uint64_t get_population_seed(const uint64_t world_seed, const int block_x, const int block_z) {
//     uint64_t xr;
//     setSeed(&xr, world_seed);
//     uint64_t a = nextLong(&xr) | 1;
//     uint64_t b = nextLong(&xr) | 1;
    
//     uint64_t population_seed = (uint64_t)block_x * a + (uint64_t)block_z * b ^ world_seed;
//     return population_seed;
// }

uint64_t get_decorator_seed(const uint64_t world_seed, const int block_x, const int block_z, const uint32_t salt) {
    uint64_t population_seed = get_population_seed(world_seed, block_x, block_z);
    return population_seed + salt;
}

uint64_t get_carver_seed(uint64_t world_seed, int chunk_x, int chunk_z) {
    uint64_t rand;
    setSeed(&rand, world_seed);

    uint64_t long6 = nextLong(&rand);
    uint64_t long8 = nextLong(&rand);
    uint64_t long10 = chunk_x * long6 ^ chunk_z * long8 ^ world_seed;
    
    return long10;
}

uint64_t get_position_seed(int x, int y, int z) {
    uint64_t l = (uint64_t)(x * 3129871) ^ (uint64_t)z * (uint64_t)116129781 ^ (uint64_t)y;
    uint64_t seed = ((l * l * (uint64_t)42317861 + l * (uint64_t)11) >> 16) ^ 0x5deece66dUL;
    return seed;
}

// void enchant_with_levels_java(uint64_t* rand, int level, const int ench_val) {
//     level += 1 + nextInt(rand, ench_val / 4 + 1) + nextInt(rand, ench_val / 4 + 1);
//     float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0f) * 0.15f;
//     level = clamp((int)round((float)level + (float)level * amplifier), 1, INT_MAX);
    
//     nextInt(rand, 1);
//     while (nextInt(rand, 50) <= level) {
//         nextInt(rand, 1);
//         level /= 2;
//     }
// }