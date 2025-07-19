#pragma once

#include "cubiomes/finders.h"

#ifndef BASTION_H
#define BASTION_H 

typedef struct {
    int id;
    int min;
    int max;
    int weight;
    int enchant;
    int broken;
} BastionLootEntry;


#ifdef __cplusplus
extern "C"
{
#endif

int get_bastion_triple_loot(const uint64_t seed, const Pos* pos, int* loot);
int get_bastion_chest_loot(uint64_t *rand, int* loot, int offset);
int get_bastion_ramparts_loot(const uint64_t seed, Pos* pos1, Pos* pos2, int* loot);
int get_stables_loot(const uint64_t seed, Pos* pos1, Pos* pos2, Pos* pos3, int* loot);

#ifdef __cplusplus
}
#endif

#endif