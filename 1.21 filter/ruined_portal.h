#pragma once

#include "utilities.h"
#include "cubiomes/finders.h"

#ifndef RUINED_PORTAL_H
#define RUINED_PORTAL_H 

typedef enum {
    portal_1,
    portal_2,
    portal_3,
    portal_4,
    portal_5,
    portal_6,
    portal_7,
    portal_8,
    portal_9,
    portal_10,
    giant_portal_1,
    giant_portal_2,
    giant_portal_3,
} RuinedPortalType;

typedef struct {
    int buried;
    RuinedPortalType type;
    int rotation;
    int mirror;
    int required_obsidian;
    Pos3 position;
    Pos3 pivot;
} RuinedPortal;

typedef struct {
    int id;
    int min;
    int max;
    int weight;
    int roll_amount;
    int enchant;
} RuinedPortalLootEntry;

void get_ruined_portal(RuinedPortal* portal, uint64_t world_seed, int chunk_x, int chunk_z);
int obsidian_frame(RuinedPortal* portal);
int get_ruined_portal_loot(const uint64_t seed, const Pos* portal_pos, int* loot, int salt);
void get_ruined_portal_chest_pos(const RuinedPortal* portal, Pos3* pos);

#endif