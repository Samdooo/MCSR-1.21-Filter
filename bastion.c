#include "bastion.h"
#include "cubiomes.h"
#include "utilities.h"
#include "enchantment.h"

extern Enchantment BOOK_ENCHANTMENTS[];

extern int TOOL_ENCHANTMENTS[];
extern int AXE_ENCHANTMENTS[];
extern int SWORD_ENCHANTMENTS[];
extern int BOOTS_ENCHANTMENTS[];
extern int LEGGINGS_ENCHANTMENTS[];
extern int CHESTPLATE_ENCHANTMENTS[];
extern int HELMET_ENCHANTMENTS[];
extern int CROSSBOW_ENCHANTMENTS[];

extern int TOOL_ENCHANTMENTS_NUMBER;
extern int AXE_ENCHANTMENTS_NUMBER;
extern int SWORD_ENCHANTMENTS_NUMBER;
extern int BOOTS_ENCHANTMENTS_NUMBER;
extern int LEGGINGS_ENCHANTMENTS_NUMBER;
extern int CHESTPLATE_ENCHANTMENTS_NUMBER;
extern int HELMET_ENCHANTMENTS_NUMBER;
extern int CROSSBOW_ENCHANTMENTS_NUMBER;

BastionLootEntry loot_table[] = {
//id,min,max,weight, enchant,break;
	{ 0, 1, 1, 6, 1, 0 }, // "minecraft:diamond_pickaxe",
	{ 1, 1, 1, 6, 0, 0 }, // "minecraft:diamond_shovel",
	{ 2, 1, 1, 6, 1, 1 }, // "minecraft:crossbow",
	{ 3, 1, 1, 12, 0, 0 }, // "minecraft:ancient_debris",
	{ 4, 1, 1, 4, 0, 0 }, // "minecraft:netherite_scrap",
	{ 5, 10, 22, 10, 0, 0 }, // "minecraft:spectral_arrow",
	{ 6, 1, 1, 9, 0, 0}, // "minecraft:piglin_banner_pattern",
	{ 7, 1, 1, 5, 0, 0}, // "minecraft:music_disc_pigstep",
	{ 8, 6, 17, 12, 0, 0 }, // "minecraft:golden_carrot",
	{ 9, 1, 1, 9, 0, 0 }, // "minecraft:golden_apple",
	{ 10, 1, 1, 10, 1, 0 }, // "minecraft:book",

	{ 11, 1, 1, 2, 1, 1 }, // "minecraft:iron_sword",
	{ 12, 1, 1, 2, 0, 0 }, // "minecraft:iron_block",
	{ 13, 1, 1, 1, 1, 0 }, // "minecraft:golden_boots",
	{ 14, 1, 1, 1, 1, 0 }, // "minecraft:golden_axe",
	{ 15, 1, 1, 2, 0, 0 }, // "minecraft:gold_block",
	{ 16, 1, 1, 1, 0, 0 }, // "minecraft:crossbow",
	{ 17, 1, 6, 2, 0, 0 }, // "minecraft:gold_ingot",
	{ 18, 1, 6, 2, 0, 0 }, // "minecraft:iron_ingot",
	{ 19, 1, 1, 1, 0, 0 }, // "minecraft:golden_sword",
	{ 20, 1, 1, 1, 0, 0 }, // "minecraft:golden_chestplate",
	{ 21, 1, 1, 1, 0, 0 }, // "minecraft:golden_helmet",
	{ 22, 1, 1, 1, 0, 0 }, // "minecraft:golden_leggings",
	{ 23, 1, 1, 1, 0, 0 }, // "minecraft:golden_boots",
	{ 24, 1, 5, 2, 0, 0 }, // "minecraft:crying_obsidian",
	
	{ 25, 1, 5, 2, 0, 0 }, // "minecraft:gilded_blackstone",
	{ 26, 2, 10, 1, 0, 0 }, // "minecraft:chain",
	{ 27, 2, 6, 2, 0, 0 }, // "minecraft:magma_cream",
	{ 28, 3, 6, 1, 0, 0 }, // "minecraft:bone_block",
	{ 29, 2, 8, 1, 0, 0 }, // "minecraft:iron_nugget",
	{ 30, 4, 6, 1, 0, 0 }, // "minecraft:obsidian",
	{ 31, 2, 8, 1, 0, 0 }, // "minecraft:gold_nugget",
	{ 32, 4, 6, 1, 0, 0 }, // "minecraft:string",
	{ 33, 5, 17, 2, 0, 0 }, // "minecraft:arrow",
	{ 34, 1, 1, 1, 0, 0 }, // "minecraft:cooked_porkchop",
	
	{ 35, 1, 1, 12, 0, 0 }, // "minecraft:snout_armor_trim_smithing_template",
	
	{ 36, 1, 1, 10, 0, 0 }, // "minecraft:netherite_upgrade_smithing_template"
};

const int entries_1 = 11;
const int entries_2 = 14;
const int entries_3 = 10;

const int sum_of_weight_1 = 89;
const int sum_of_weight_2 = 20;
const int sum_of_weight_3 = 13;

int get_bastion_chest_loot(uint64_t *rand, int *loot, int offset) {
	// ----------------------- JEDEN // ------------ //
	for (int i = offset; i < 1 + offset; ++i) {
		loot[4 * i + 0] = 0; // ID
        loot[4 * i + 1] = 0; // Amount
        loot[4 * i + 2] = 0; // Enchantment ID
        loot[4 * i + 3] = 0; // Enchantment level
        
        int weight = nextInt(rand, sum_of_weight_1) + 1;
        int id = 0;

        while (weight > 0) {
            weight -= loot_table[id].weight;
            ++id;
        }
        --id;
        
        int amount = 1;
        if (loot_table[id].max != 1) {
            amount = nextInt(rand, loot_table[id].max - loot_table[id].min + 1) + loot_table[id].min;
        }

        loot[4 * i + 0] = id;
        loot[4 * i + 1] = amount;

        if (loot_table[id].broken) {
        	nextFloat(rand);
        }

        if (loot_table[id].enchant) {
			int enchantment_id = -1;
			int level = 1;

        	switch (id) {
        		case 0: {
					enchantment_id = TOOL_ENCHANTMENTS[nextInt(rand, TOOL_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(rand, max_level) + 1;
                    }
        		} break;

        		case 2: {
					enchantment_id = CROSSBOW_ENCHANTMENTS[nextInt(rand, CROSSBOW_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(rand, max_level) + 1;
                    }
        		} break;

        		case 10: {
        			enchantment_id = -1;
                    int max_level = 3;
                    nextInt(rand, 0);
                    level = nextInt(rand, max_level) + 1;
        		} break;

        		default: break;
        	}
        
			loot[4 * i + 2] = enchantment_id;
			loot[4 * i + 3] = level;
        }
	}

	// ----------------------- DWA // ------------ //
	for (int i = 1 + offset; i < 3 + offset; ++i) {
		loot[4 * i + 0] = 0; // ID
        loot[4 * i + 1] = 0; // Amount
        loot[4 * i + 2] = 0; // Enchantment ID
        loot[4 * i + 3] = 0; // Enchantment level
        
        int weight = nextInt(rand, sum_of_weight_2) + 1;
        int id = entries_1;

        while (weight > 0) {
            weight -= loot_table[id].weight;
            ++id;
        }
        --id;
        
        int amount = 1;
        if (loot_table[id].max != 1) {
            amount = nextInt(rand, loot_table[id].max - loot_table[id].min + 1) + loot_table[id].min;
        }

        loot[4 * i + 0] = id;
        loot[4 * i + 1] = amount;

        if (loot_table[id].broken) {
        	nextFloat(rand);
        }

        if (loot_table[id].enchant) {
        	int enchantment_id = -1;
			int level = 1;

        	switch (id) {
        		case 11: {
					enchantment_id = SWORD_ENCHANTMENTS[nextInt(rand, SWORD_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(rand, max_level) + 1;
                    }
        		} break;

        		case 13: {
        			enchantment_id = -1;
                    int max_level = 3;
                  	nextInt(rand, 3);
                    level = nextInt(rand, max_level) + 1;
        		} break;

        		case 14: {
					enchantment_id = AXE_ENCHANTMENTS[nextInt(rand, AXE_ENCHANTMENTS_NUMBER)];
                    int max_level = BOOK_ENCHANTMENTS[enchantment_id].max_level;

                    if (max_level > 1) {
                        level = nextInt(rand, max_level) + 1;
                    }
        		} break;

        		default: break;
        	}

			loot[4 * i + 2] = enchantment_id;
			loot[4 * i + 3] = level;
        }
	}

	// ----------------------- TRSYYYYZY // ------------ //
	int entries = nextInt(rand, 2) + 3;
	for (int i = 3 + offset; i < entries + 3 + offset; ++i) {
		loot[4 * i + 0] = 0; // ID
        loot[4 * i + 1] = 0; // Amount
        loot[4 * i + 2] = 0; // Enchantment ID
        loot[4 * i + 3] = 0; // Enchantment level
        
        int weight = nextInt(rand, sum_of_weight_3) + 1;
        int id = entries_1 + entries_2;

        while (weight > 0) {
            weight -= loot_table[id].weight;
            ++id;
        }
        --id;
        
        int amount = 1;
        if (loot_table[id].max != 1) {
            amount = nextInt(rand, loot_table[id].max - loot_table[id].min + 1) + loot_table[id].min;
        }

        loot[4 * i + 0] = id;
        loot[4 * i + 1] = amount;
	}

	return entries + 3 + offset;
}

int get_bastion_triple_loot(const uint64_t seed, const Pos* pos, int* loot) {
	Xoroshiro xr;
    uint64_t rand;
    uint64_t decorator_seed;
    uint64_t loot_seed;

    decorator_seed = get_decorator_seed(seed, pos->x, pos->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    int stuff = 0;

    for (int i = 0; i < 5; ++i) {
    	loot_seed = xNextLongJ(&xr);
    	if (i == 0 || i == 1) continue;
    	// printf("%ld\n", loot_seed);
		setSeed(&rand, loot_seed);	
		stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    return stuff;
}

int get_bastion_ramparts_loot(const uint64_t seed, Pos* pos1, Pos* pos2, int* loot) {
	Xoroshiro xr;
    uint64_t rand;
    uint64_t decorator_seed;
    uint64_t loot_seed;

    decorator_seed = get_decorator_seed(seed, pos1->x, pos1->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    int stuff = 0;

    for (int i = 0; i < 5; ++i) {
    	loot_seed = xNextLongJ(&xr);
    	if (i == 0 || i == 1) continue;
    	// printf("%ld\n", loot_seed);
		setSeed(&rand, loot_seed);	
		stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    decorator_seed = get_decorator_seed(seed, pos2->x, pos2->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    for (int i = 0; i < 3; ++i) {
    	loot_seed = xNextLongJ(&xr);
		setSeed(&rand, loot_seed);	
		stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    return stuff;
}

int get_stables_loot(const uint64_t seed, Pos* pos1, Pos* pos2, Pos* pos3, int* loot) {
    Xoroshiro xr;
    uint64_t rand;
    uint64_t decorator_seed;
    uint64_t loot_seed;

    int stuff = 0;

    decorator_seed = get_decorator_seed(seed, pos1->x, pos1->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    for (int i = 0; i < 3; ++i) {
        loot_seed = xNextLongJ(&xr);
        setSeed(&rand, loot_seed);  
        stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    decorator_seed = get_decorator_seed(seed, pos2->x, pos2->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    for (int i = 0; i < 3; ++i) {
        loot_seed = xNextLongJ(&xr);
        setSeed(&rand, loot_seed);  
        stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    decorator_seed = get_decorator_seed(seed, pos3->x, pos3->z, BASTION_SALT);
    xSetSeed(&xr, decorator_seed);

    for (int i = 0; i < 3; ++i) {
        loot_seed = xNextLongJ(&xr);
        setSeed(&rand, loot_seed);  
        stuff = get_bastion_chest_loot(&rand, loot, stuff);
    }

    return stuff;
}