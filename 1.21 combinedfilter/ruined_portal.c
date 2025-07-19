#include "utilities.h"
#include "ruined_portal.h"
#include "cubiomes/rng.h"
#include "enchantment.h"

#include <stdio.h>

extern Enchantment BOOK_ENCHANTMENTS[];

extern int TOOL_ENCHANTMENTS[];
extern int AXE_ENCHANTMENTS[];
extern int SWORD_ENCHANTMENTS[];
extern int BOOTS_ENCHANTMENTS[];
extern int LEGGINGS_ENCHANTMENTS[];
extern int CHESTPLATE_ENCHANTMENTS[];
extern int HELMET_ENCHANTMENTS[];

extern int TOOL_ENCHANTMENTS_NUMBER;
extern int AXE_ENCHANTMENTS_NUMBER;
extern int SWORD_ENCHANTMENTS_NUMBER;
extern int BOOTS_ENCHANTMENTS_NUMBER;
extern int LEGGINGS_ENCHANTMENTS_NUMBER;
extern int CHESTPLATE_ENCHANTMENTS_NUMBER;
extern int HELMET_ENCHANTMENTS_NUMBER;

#define SUM_OF_WEIGHTS 398

RuinedPortalLootEntry loot_tejbuj[] = {
    // ID, min., max., weight, roll_amount?, enchant?
    { 0, 1, 2, 40, 1, 0 }, //   obsidian
    { 1, 1, 4, 40, 1, 0 }, //   flint
    { 2, 9, 18, 40, 1, 0 }, //  iron_nugget
    { 3, 1, 1, 40, 0, 0 }, //   flint_and_steel
    { 4, 1, 1, 40, 0, 0 }, //   fire_charge
    { 5, 1, 1, 15, 0, 0 }, //   golden_apple
    { 6, 4, 24, 15, 1, 0 }, //  gold_nugget
    { 7, 1, 1, 15, 0, 1 }, //   golden_sword
    { 8, 1, 1, 15, 0, 1 }, //   golden_axe
    { 9, 1, 1, 15, 0, 1 }, //   golden_hoe
    { 10, 1, 1, 15, 0, 1 }, //  golden_shovel
    { 11, 1, 1, 15, 0, 1 }, //  golden_pickaxe
    { 12, 1, 1, 15, 0, 1 }, //  golden_boots
    { 13, 1, 1, 15, 0, 1 }, //  golden_chestplate
    { 14, 1, 1, 15, 0, 1 }, //  golden_helmet
    { 15, 1, 1, 15, 0, 1 }, //  golden_leggings
    { 16, 4, 12, 5, 1, 0 }, //  glistering_melon_slice
    { 17, 1, 1, 5, 0, 0 }, //   golden_horse_armour
    { 18, 1, 1, 5, 0, 0 }, //   light_weighted_pressure_plate
    { 19, 4, 12, 5, 1, 0 }, //  golden_carrot
    { 20, 1, 1, 5, 0, 0 }, //   clock
    { 21, 2, 8, 5, 1, 0 }, //   gold_ingot
    { 22, 1, 1, 1, 0, 0 }, //   bell
    { 23, 1, 1, 1, 0, 0 }, //   enchanted_golden_apple
    { 24, 1, 2, 1, 1, 0 }, //   gold_block
};


void get_ruined_portal(RuinedPortal* portal, uint64_t world_seed, int chunk_x, int chunk_z) {
    // Tylko zwykłe portale.

    uint64_t rand;
    uint64_t carver_seed = get_carver_seed(world_seed, chunk_x, chunk_z);

    setSeed(&rand, carver_seed);
    if (nextFloat(&rand) < 0.5f) {
        portal->buried = 1;
    } else {
        portal->buried = 0;
        nextFloat(&rand);    
    }


    RuinedPortalType type;
    if (nextFloat(&rand) < 0.05f) {
        type = 10 + nextInt(&rand, 3);
    } else {
        type = nextInt(&rand, 10);
    }

    Pos3 pivot = {0, 0, 0};
    if (type == giant_portal_1) { pivot.x = 11 >> 1;  pivot.z = 16 >> 1;  portal->required_obsidian = 5; } 
    if (type == giant_portal_2) { pivot.x = 11 >> 1;  pivot.z = 16 >> 1;  portal->required_obsidian = 5; } 
    if (type == giant_portal_3) { pivot.x = 16 >> 1;  pivot.z = 16 >> 1;  portal->required_obsidian = 5; } 
    if (type == portal_1)       { pivot.x = 6 >> 1;   pivot.z = 6 >> 1;   portal->required_obsidian = 2; } 
    if (type == portal_2)       { pivot.x = 9 >> 1;   pivot.z = 9 >> 1;   portal->required_obsidian = 4; } 
    if (type == portal_3)       { pivot.x = 8 >> 1;   pivot.z = 9 >> 1;   portal->required_obsidian = 4; } 
    if (type == portal_4)       { pivot.x = 8 >> 1;   pivot.z = 9 >> 1;   portal->required_obsidian = 3; } 
    if (type == portal_5)       { pivot.x = 10 >> 1;  pivot.z = 7 >> 1;   portal->required_obsidian = 5; } 
    if (type == portal_6)       { pivot.x = 5 >> 1;   pivot.z = 7 >> 1;   portal->required_obsidian = 1; } 
    if (type == portal_7)       { pivot.x = 9 >> 1;   pivot.z = 9 >> 1;   portal->required_obsidian = 1; } 
    if (type == portal_8)       { pivot.x = 14 >> 1;  pivot.z = 9 >> 1;   portal->required_obsidian = 3; } 
    if (type == portal_9)       { pivot.x = 10 >> 1;  pivot.z = 9 >> 1;   portal->required_obsidian = 2; } 
    if (type == portal_10)      { pivot.x = 12 >> 1;  pivot.z = 10 >> 1;  portal->required_obsidian = 7; } 

    portal->type = type;
    portal->rotation = nextInt(&rand, 4);
    portal->mirror = nextFloat(&rand) < 0.5f;
    portal->pivot.x = pivot.x;
    portal->pivot.z = pivot.z;
    portal->position.x = chunk_x << 4;
    portal->position.z = chunk_z << 4;
}

int obsidian_frame(RuinedPortal* portal) {
    Pos3 minimal_portal_frame[11];
    int blocks_to_check;

    switch (portal->type) {
        case giant_portal_1: {
            minimal_portal_frame[0] = (Pos3){ 5, 3, 9 };
            minimal_portal_frame[1] = (Pos3){ 5, 3, 10 };
            minimal_portal_frame[2] = (Pos3){ 5, 4, 11 };
            minimal_portal_frame[3] = (Pos3){ 5, 5, 11 };
            minimal_portal_frame[4] = (Pos3){ 5, 6, 11 };
            blocks_to_check = 5;
            break;
        }
        
        case giant_portal_2: {
            minimal_portal_frame[0] = (Pos3){ 5, 3, 9 };
            minimal_portal_frame[1] = (Pos3){ 5, 3, 10 };
            minimal_portal_frame[2] = (Pos3){ 5, 4, 11 };
            minimal_portal_frame[3] = (Pos3){ 5, 5, 11 };
            minimal_portal_frame[4] = (Pos3){ 5, 6, 11 };
            blocks_to_check = 5;
            break;
        }

        case giant_portal_3: {
            minimal_portal_frame[0] = (Pos3){ 5, 3, 9 };
            minimal_portal_frame[1] = (Pos3){ 5, 3, 10 };
            minimal_portal_frame[2] = (Pos3){ 5, 4, 11 };
            minimal_portal_frame[3] = (Pos3){ 5, 5, 11 };
            minimal_portal_frame[4] = (Pos3){ 5, 6, 11 };
            blocks_to_check = 5;
            break;
        }

        case portal_1: {
            minimal_portal_frame[0] = (Pos3){ 3, 2, 2 };
            minimal_portal_frame[1] = (Pos3){ 3, 2, 3 };
            minimal_portal_frame[2] = (Pos3){ 3, 3, 1 };
            minimal_portal_frame[3] = (Pos3){ 3, 3, 4 };
            minimal_portal_frame[4] = (Pos3){ 3, 4, 1 };
            minimal_portal_frame[5] = (Pos3){ 3, 5, 1 };
            minimal_portal_frame[6] = (Pos3){ 3, 6, 2 };
            minimal_portal_frame[7] = (Pos3){ 3, 6, 3 };
            blocks_to_check = 8;
            break;
        }

        case portal_2: {
            minimal_portal_frame[0] = (Pos3){ 5, 5, 2 };
            minimal_portal_frame[1] = (Pos3){ 5, 6, 2 };
            minimal_portal_frame[2] = (Pos3){ 5, 7, 2 };
            minimal_portal_frame[3] = (Pos3){ 5, 7, 5 };
            minimal_portal_frame[4] = (Pos3){ 5, 8, 3 };
            minimal_portal_frame[5] = (Pos3){ 5, 8, 4 };
            blocks_to_check = 6;
            break;
        }

        case portal_3: {
            minimal_portal_frame[0] = (Pos3){ 4, 3, 3 };
            minimal_portal_frame[1] = (Pos3){ 4, 3, 4 };
            minimal_portal_frame[2] = (Pos3){ 4, 4, 5 };
            minimal_portal_frame[3] = (Pos3){ 4, 5, 5 };
            minimal_portal_frame[4] = (Pos3){ 4, 6, 5 };
            minimal_portal_frame[5] = (Pos3){ 4, 7, 4 };
            blocks_to_check = 6;
            break;
        }

        case portal_4: {
            minimal_portal_frame[0] = (Pos3){ 4, 3, 3 };
            minimal_portal_frame[1] = (Pos3){ 4, 3, 4 };
            minimal_portal_frame[2] = (Pos3){ 4, 4, 2 };
            minimal_portal_frame[3] = (Pos3){ 4, 4, 5 };
            minimal_portal_frame[4] = (Pos3){ 4, 5, 2 };
            minimal_portal_frame[5] = (Pos3){ 4, 5, 5 };
            minimal_portal_frame[6] = (Pos3){ 4, 6, 2 };
            blocks_to_check = 7;
            break;
        }

        case portal_5: {
            minimal_portal_frame[0] = (Pos3){ 2, 3, 2 };
            minimal_portal_frame[1] = (Pos3){ 2, 3, 3 };
            minimal_portal_frame[2] = (Pos3){ 2, 4, 4 };
            minimal_portal_frame[3] = (Pos3){ 2, 5, 4 };
            minimal_portal_frame[4] = (Pos3){ 2, 6, 4 };
            blocks_to_check = 5;
            break;
        }

        case portal_6: {
            minimal_portal_frame[0] = (Pos3){ 2, 1, 1 };
            minimal_portal_frame[1] = (Pos3){ 2, 1, 2 };
            minimal_portal_frame[2] = (Pos3){ 2, 1, 3 };
            minimal_portal_frame[3] = (Pos3){ 2, 2, 0 };
            minimal_portal_frame[4] = (Pos3){ 2, 2, 4 };
            minimal_portal_frame[5] = (Pos3){ 2, 3, 0 };
            minimal_portal_frame[6] = (Pos3){ 2, 3, 4 };
            minimal_portal_frame[7] = (Pos3){ 2, 4, 0 };
            minimal_portal_frame[8] = (Pos3){ 2, 4, 4 };
            minimal_portal_frame[9] = (Pos3){ 2, 5, 1 };
            minimal_portal_frame[10] = (Pos3){ 2, 5, 3 };
            blocks_to_check = 11;
            break;
        }

        case portal_7: {
            minimal_portal_frame[0] = (Pos3){ 3, 0, 3 };
            minimal_portal_frame[1] = (Pos3){ 3, 0, 4 };
            minimal_portal_frame[2] = (Pos3){ 3, 1, 2 };
            minimal_portal_frame[3] = (Pos3){ 3, 1, 5 };
            minimal_portal_frame[4] = (Pos3){ 3, 2, 2 };
            minimal_portal_frame[5] = (Pos3){ 3, 2, 5 };
            minimal_portal_frame[6] = (Pos3){ 3, 3, 2 };
            minimal_portal_frame[7] = (Pos3){ 3, 4, 3 };
            minimal_portal_frame[8] = (Pos3){ 3, 4, 4 };
            blocks_to_check = 9;
            break;
        }

        case portal_8: {
            minimal_portal_frame[0] = (Pos3){ 5, 3, 3 };
            minimal_portal_frame[1] = (Pos3){ 5, 3, 4 };
            minimal_portal_frame[2] = (Pos3){ 5, 3, 5 };
            minimal_portal_frame[3] = (Pos3){ 5, 4, 2 };
            minimal_portal_frame[4] = (Pos3){ 5, 4, 6 };
            minimal_portal_frame[5] = (Pos3){ 5, 5, 2 };
            minimal_portal_frame[6] = (Pos3){ 5, 5, 6 };
            minimal_portal_frame[7] = (Pos3){ 5, 6, 2 };
            minimal_portal_frame[8] = (Pos3){ 5, 6, 6 };
            blocks_to_check = 9;
            break;
        }

        case portal_9: {
            minimal_portal_frame[0] = (Pos3){ 4, 1, 4 };
            minimal_portal_frame[1] = (Pos3){ 4, 1, 5 };
            minimal_portal_frame[2] = (Pos3){ 4, 2, 3 };
            minimal_portal_frame[3] = (Pos3){ 4, 2, 6 };
            minimal_portal_frame[4] = (Pos3){ 4, 3, 6 };
            minimal_portal_frame[5] = (Pos3){ 4, 4, 6 };
            minimal_portal_frame[6] = (Pos3){ 4, 5, 4 };
            minimal_portal_frame[7] = (Pos3){ 4, 5, 5 };
            blocks_to_check = 8;
            break;
        }

        case portal_10: {
            minimal_portal_frame[0] = (Pos3){ 3, 1, 4 };
            minimal_portal_frame[1] = (Pos3){ 3, 1, 5 };
            minimal_portal_frame[2] = (Pos3){ 3, 2, 3 };
            blocks_to_check = 3;
            break;
        }
        
        default:
            return 0;
    }

    for (int i = 0; i < blocks_to_check; ++i) {
        Pos3 pos = minimal_portal_frame[i];

        if (portal->mirror == 0) {
            pos.x *= -1;
        }

        pos.x -= portal->pivot.x;
        pos.z -= portal->pivot.z;

        int temp_x = pos.x;
        int temp_z = pos.z;
        if (portal->rotation == 1){
            pos.x = -temp_z;
            pos.z = temp_x;
        } else if (portal->rotation == 2){
            pos.x = -temp_x;
            pos.z = -temp_z;
        } else if (portal->rotation == 3){
            pos.x = temp_z;
            pos.z = -temp_x;
        }

        pos.x += portal->pivot.x;
        pos.z += portal->pivot.z;

        pos.x += portal->position.x;
        pos.z += portal->position.z;

        uint64_t rand;
        setSeed(&rand, get_position_seed(pos.x, pos.y, pos.z));

        if (nextFloat(&rand) >= 0.15f) {
            return 0;
        }
    }

    return 1;
}

void get_ruined_portal_chest_pos(const RuinedPortal* portal, Pos3* pos) {
    Pos3 loot_pos;
    
    switch (portal->type) {
        case giant_portal_1: loot_pos = (Pos3){ 4, 3, 3 }; break;
        case giant_portal_2: loot_pos = (Pos3){ 9, 1, 9 }; break;
        case giant_portal_3: loot_pos = (Pos3){ 9, 2, 3 }; break;
        case portal_1: loot_pos = (Pos3){ 2, 2, 0 }; break;
        case portal_2: loot_pos = (Pos3){ 8, 2, 6 }; break;
        case portal_3: loot_pos = (Pos3){ 3, 3, 6 }; break;
        case portal_4: loot_pos = (Pos3){ 3, 3, 2 }; break;
        case portal_5: loot_pos = (Pos3){ 4, 3, 2 }; break;
        case portal_6: loot_pos = (Pos3){ 1, 1, 4 }; break;
        case portal_7: loot_pos = (Pos3){ 0, 1, 2 }; break;
        case portal_8: loot_pos = (Pos3){ 4, 4, 2 }; break;
        case portal_9: loot_pos = (Pos3){ 4, 1, 0 }; break;
        case portal_10: loot_pos = (Pos3){ 2, 1, 7 }; break;
        default: return;
    }

    pos->x = loot_pos.x;
    pos->z = loot_pos.z;

    if (portal->mirror == 0) {
        pos->x *= -1;
    }

    pos->x -= portal->pivot.x;
    pos->z -= portal->pivot.z;

    int temp_x = pos->x;
    int temp_z = pos->z;
    if (portal->rotation == 1) {
        pos->x = -temp_z;
        pos->z = temp_x;
    } else if (portal->rotation == 2) {
        pos->x = -temp_x;
        pos->z = -temp_z;
    } else if (portal->rotation == 3) {
        pos->x = temp_z;
        pos->z = -temp_x;
    }

    pos->x += portal->pivot.x;
    pos->z += portal->pivot.z;

    pos->x += portal->position.x;
    pos->z += portal->position.z;
}

int get_ruined_portal_loot(const uint64_t seed, const Pos* portal_pos, int* loot, int salt) {
    Xoroshiro xr;
    uint64_t rand;
    uint64_t decorator_seed = get_decorator_seed(seed, portal_pos->x, portal_pos->z, salt);
    xSetSeed(&xr, decorator_seed);
    uint64_t loot_seed = xNextLongJ(&xr);
    setSeed(&rand, loot_seed);

    // printf("%lld\n", loot_seed);
    // printf("(%d, %d)\n", portal_pos->x, portal_pos->z);

    int entries = nextInt(&rand, 5) + 4;
    for (int i = 0; i < entries; ++i) {
        loot[4 * i + 0] = 0; // ID
        loot[4 * i + 1] = 0; // Amount
        loot[4 * i + 2] = 0; // Enchantment ID
        loot[4 * i + 3] = 0; // Enchantment level
        
        int weight = nextInt(&rand, SUM_OF_WEIGHTS) + 1;
        int id = 0;

        while (weight > 0) {
            weight -= loot_tejbuj[id].weight;
            ++id;
        }
        --id;
        
        int amount = 1;
        if (loot_tejbuj[id].roll_amount) {
            amount = nextInt(&rand, loot_tejbuj[id].max - loot_tejbuj[id].min + 1) + loot_tejbuj[id].min;
        }

        loot[4 * i + 0] = id;
        loot[4 * i + 1] = amount;

        if (loot_tejbuj[id].enchant) {
            int enchantment_id = 0;
            int level = 1;

            switch (id) {
                case 7: { // Sword
                    enchantment_id = SWORD_ENCHANTMENTS[nextInt(&rand, SWORD_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }

                case 8: { // Axe
                    enchantment_id = AXE_ENCHANTMENTS[nextInt(&rand, AXE_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }

                case 9: 
                case 10: 
                case 11: { // Other tools
                    enchantment_id = TOOL_ENCHANTMENTS[nextInt(&rand, TOOL_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }
                
                case 12: { // Boots
                    enchantment_id = BOOTS_ENCHANTMENTS[nextInt(&rand, BOOTS_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }

                case 13: { // Chestplate
                    enchantment_id = CHESTPLATE_ENCHANTMENTS[nextInt(&rand, CHESTPLATE_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }

                case 14: { // Helmet
                    enchantment_id = HELMET_ENCHANTMENTS[nextInt(&rand, HELMET_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }

                case 15: { // Leggings
                    enchantment_id = LEGGINGS_ENCHANTMENTS[nextInt(&rand, LEGGINGS_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(&rand, max_level) + 1;
                    }

                    break;
                }
                
                
                default: {

                    break;
                }
            }

            loot[4 * i + 2] = enchantment_id;
            loot[4 * i + 3] = level;
        }
    }

    return entries;
}

// { 7, 1, 1, 15, 0, 1 }, // golden sword
// { 8, 1, 1, 15, 0, 1 }, // golden axe
// { 9, 1, 1, 15, 0, 1 }, // golden hoe
// { 10, 1, 1, 15, 0, 1 }, // golden shovel
// { 11, 1, 1, 15, 0, 1 }, // golden pickaxe
// { 12, 1, 1, 15, 0, 1 }, // golden boots
// { 13, 1, 1, 15, 0, 1 }, // golden chestplate
// { 14, 1, 1, 15, 0, 1 }, // golden helmet
// { 15, 1, 1, 15, 0, 1 }, // golden leggings