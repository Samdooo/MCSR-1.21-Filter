#pragma once

#include <inttypes.h>

#ifndef UTILITIES_H
#define UTILITIES_H

#define VERSION MC_1_21
#define RUINED_PORTAL_SALT 40010
#define BASTION_SALT 40000

typedef struct {
    int min;
    int max;
} Iterval;

// typedef enum {
//     SWORD,
//     PICKAXE,
//     AXE,
//     SHOVEL,
//     HOE,
//     HELMET,
//     CHESTPLATE,
//     LEGGINGS,
//     BOOTS,
// } EquipmentType;

typedef enum {
    DEPTH_STRIDER_MASK  = 0b1000000000,
    RESPIRATION_MASK    = 0b0100000000,
    AQUA_AFFINITY_MASK  = 0b0010000000,
    LOOTING_MASK        = 0b0001000000,
    SMITE_MASK          = 0b0000100000,
    PICKAXE_MASK        = 0b0000010000,
    UNBREAKING_MASK     = 0b0000001000,
    MENDING_MASK        = 0b0000000100,
    CHANNELING_MASK     = 0b0000000010,
    PIERCING_MASK       = 0b0000000001,
} RequiredEnchantmentsMasks;

// 1111110111
// 1111111011
// 1111111101

// 1110010111
// 1110011011

// Depth Strider 2+
// Respiration 1+
// Aqua Affinity
// Looting 3
// Smite 4 Diamond / 5 Iron
// Unbreaking 3
// Mending
// Channeling
// Piercing 4

#ifdef __cplusplus
extern "C"
{
#endif

int clamp(const int v, const int mn, const int mx);
int max(int x, int y);
int min(int x, int y);
uint64_t get_population_seed(const uint64_t world_seed, const int block_x, const int block_z);
uint64_t get_decorator_seed(const uint64_t world_seed, const int block_x, const int block_z, const uint32_t salt);
uint64_t get_carver_seed(uint64_t world_seed, int chunk_x, int chunk_z);
uint64_t get_position_seed(int x, int y, int z);
void enchant_with_levels_java(uint64_t* rand, int level, const int ench_val);

#ifdef __cplusplus
}
#endif

#endif