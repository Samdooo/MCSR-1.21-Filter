#include <stdio.h>
#include <omp.h>
#include <inttypes.h>

#include "cubiomes.h"
#include "bastion.h"
#include "ruined_portal.h"
#include "MiLTSU.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Invalid arguments, length.\n");
        return 1;
    }

    int part = strtoull(argv[1], NULL, 10);
    if (part < 1 || part > 16) {
        printf("Invalid arguments, content.\n");
        return 1;
    }

	#pragma omp parallel num_threads(4)
	{
		FILE* input = fopen("input", "r");
		if (input == NULL) {
		    printf("Couldn't open the input file.\n");
		    exit(1);
		}

		Generator g;
		setupGenerator(&g, VERSION, 0);

		uint64_t structure_seed, world_seed, j, rnd;
		Pos bastion_pos, portal_pos;

		int rotation;
		int loot[1024];
		int id, count, enchantment_id, enchantment_level;
		int obsidian;
		RuinedPortal portal;
		int portal_obsidian, charges, nether_charges, flint_and_steel, flint, sword, axe;
		Pos3 portal_chest_pos;
		int entries;
		Xoroshiro xoro;
		int string;
		int pearls, gold_ingots, biome_id;
		StructureVariant portal_variant;

		int five_chests_x, five_chests_z, three_chests_x, three_chests_z;
		int iron_nuggets, iron_ingots, salt;
		char* blaze_str = "minecraft:entities/blaze";
		char* bartering_str = "minecraft:gameplay/piglin_bartering";
		int rods, i;
		int MAX_INGOTS = 9;
		const int required_pearls = 6;
		const int required_rods = 2;
		// int part = 16;
		Pos spawn;

		char chunk[128];
		while(fgets(chunk, sizeof(chunk), input) != NULL) {
			structure_seed = strtoull(chunk, NULL, 10);

			getStructurePos(Ruined_Portal, MC_1_21, structure_seed, 0, 0, &portal_pos);
			getStructurePos(Bastion, MC_1_21, structure_seed, 0, 0, &bastion_pos);

			setSeed(&rnd, structure_seed);
			rnd = (nextLong(&rnd) * (bastion_pos.x >> 4)) ^ (nextLong(&rnd) * (bastion_pos.z >> 4)) ^ structure_seed;

			setSeed(&rnd, rnd);
			rotation = nextInt(&rnd, 4);

			switch (rotation) {
		        case 0: { // -> dodatnie z
		            five_chests_x = bastion_pos.x - 16;
		            five_chests_z = bastion_pos.z + 16;

					three_chests_x = five_chests_x;
		            three_chests_z = five_chests_z + 16;
		        } break;

		        case 1: { // -> ujemny x
		            five_chests_x = bastion_pos.x - 32;
		            five_chests_z = bastion_pos.z - 16;

					three_chests_x = five_chests_x - 16;
		            three_chests_z = five_chests_z;
		        } break;
		        
		        case 2: { // -> ujemne z
		            five_chests_x = bastion_pos.x;
		            five_chests_z = bastion_pos.z - 32;

					three_chests_x = five_chests_x;
		            three_chests_z = five_chests_z - 16;
		        } break;
		        
		        case 3: { // -> dodatni x 
		            five_chests_x = bastion_pos.x + 16;
		            five_chests_z = bastion_pos.z;

					three_chests_x = five_chests_x + 16;
		            three_chests_z = five_chests_z;
		        } break;
		        
		        default:
		            break;
		    }

			get_ruined_portal(&portal, structure_seed, portal_pos.x >> 4, portal_pos.z >> 4);
			get_ruined_portal_chest_pos(&portal, &portal_chest_pos);

			#pragma omp for
			for (j = 4096 * (part - 1); j < 4096 * (part); ++j) {
				iron_nuggets = 0;
			    iron_ingots = 0;
			    gold_ingots = 0;
			    obsidian = 0;
			    string = 0;
				charges = 0;
				nether_charges = 0;
				pearls = 0;
				portal_obsidian = 0;
				flint = 0;
				flint_and_steel = 0;
				sword = 0;
				rods = 0;
				axe = 0;

				world_seed = structure_seed | (j << 48);

				xoro = getRandomSequenceXoro(world_seed, bartering_str);
		   		for (i = 0; i < MAX_INGOTS; ++i) {    	
				    BarteringOut out = nextBarteringLoot(&xoro);

				    if (out.id == 6) {
				    	pearls += out.amount;
				    }
			    }	

			    if (pearls < required_pearls) {
			    	continue;
			    }
			    pearls = 0;

				// xoro = getRandomSequenceXoro(world_seed, blaze_str);
				// for (i = 0; i < 2; ++i) {
				// 	rods += xNextInt(&xoro, 2) + round(xNextFloat(&xoro) * 3.0f);
				// }

				// if (rods < required_rods) {
				// 	continue;
				// }
				// rods = 0;

				entries = get_bastion_ramparts_loot(world_seed, &(Pos){ five_chests_x, five_chests_z }, &(Pos){ three_chests_x, three_chests_z }, loot);
				// entries = get_stables_loot(world_seed, &(Pos){ five_chests_x, five_chests_z }, &(Pos){ three_chests_x, three_chests_z }, &(Pos){ 128, 128 }, loot);
				// entries = get_bastion_triple_loot(world_seed, &(Pos){ five_chests_x, five_chests_z }, loot);
				for (i =0 ; i < entries; ++i) {
					id = loot[4*i];
					count = loot[4*i+1];
				    enchantment_id = loot[4 * i + 2];
				    enchantment_level = loot[4 * i + 3];

					if (id == 30) {
						obsidian += count;
					}

					if (id == 32) {
						string += count;
					}

					if (id == 11 && enchantment_id == 16) {
						sword = max(sword, enchantment_level);
					}

					if (id == 17) gold_ingots += count;
					if (id == 15) gold_ingots += count*9;

					if (id == 12) {
						iron_ingots += count*9;
					}

					if (id == 18) {
						iron_ingots += count;
					}

					if (id == 29) {
						iron_nuggets += count;
					}

					if (id == 14) {
						axe = 1;
					}
				}

				// if (iron_ingots < 16) {
				// 	continue;
				// }

				applySeed(&g, DIM_OVERWORLD, world_seed);
				biome_id = getBiomeAt(&g, 4, portal_pos.x >> 2, 320 >> 2, portal_pos.z >> 2);
				// biome_id = getBiomeAt(&g, 4, portal_pos.x, 320 >> 2, portal_pos.z);

				getVariant(&portal_variant, Ruined_Portal, VERSION, world_seed, portal_pos.x, portal_pos.z, biome_id);

		        // int X = (portal_pos.x + portal_variant.x + portal_variant.sx / 2) & ~15;
        		// int Z = (portal_pos.z + portal_variant.z + portal_variant.sz / 2) & ~15;
        		// printf("(%d, %d)\n", X, Z);

				if (portal_variant.biome == plains) {
					salt = 40010;
				} else if (portal_variant.biome == mountains || biome_id == cherry_grove) {
					salt = 40013;
				} else {
					continue;
				}

				// printf("%d\n", salt);

				entries = get_ruined_portal_loot(world_seed, &(Pos){ portal_chest_pos.x >> 4 << 4, portal_chest_pos.z >> 4 << 4 }, loot, salt);
				for (i = 0; i < entries; ++i) {
				    id = loot[4 * i];
				    count = loot[4 * i + 1];
				    enchantment_id = loot[4 * i + 2];
				    enchantment_level = loot[4 * i + 3];

				    if (id == 0) portal_obsidian += count;
				    if (id == 1) flint += count;
				    if (id == 3) flint_and_steel = 1;
				    if (id == 4) charges += count;
				    if (id == 7 && enchantment_id == 16) sword = max(sword, enchantment_level);

				    if (id == 21) gold_ingots += count;
				    if (id == 24) gold_ingots += count*9;

				     if (id == 2) iron_nuggets += count;
				     if (id == 8) axe = 1;
				    //  int amount = count;

	                //  switch (id) {
		            //     case 0: printf("obsidian; %d\n", amount); break;
		            //     case 1: printf("flint; %d\n", amount); break;
		            //     case 2: printf("iron_nugget; %d\n", amount); break;
		            //     case 3: printf("flint_and_steel; %d\n", amount); break;
		            //     case 4: printf("fire_charge; %d\n", amount); break;
		            //     case 5: printf("golden_apple; %d\n", amount); break;
		            //     case 6: printf("gold_nugget; %d\n", amount); break;
		            //     case 7: printf("golden_sword; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 8: printf("golden_axe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 9: printf("golden_hoe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 10: printf("golden_shovel; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 11: printf("golden_pickaxe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 12: printf("golden_boots; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 13: printf("golden_chestplate; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 14: printf("golden_helmet; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 15: printf("golden_leggings; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 16: printf("glistering_melon_slice; %d\n", amount); break;
		            //     case 17: printf("golden_horse_armour; %d\n", amount); break;
		            //     case 18: printf("light_weighted_pressure_plate; %d\n", amount); break;
		            //     case 19: printf("golden_carrot; %d\n", amount); break;
		            //     case 20: printf("clock; %d\n", amount); break;
		            //     case 21: printf("gold_ingot; %d\n", amount); break;
		            //     case 22: printf("bell; %d\n", amount); break;
		            //     case 23: printf("enchanted_golden_apple; %d\n", amount); break;
		            //     case 24: printf("gold_block; %d\n", amount); break;
		            //     default: break;
		            // }
				}

				if (portal_obsidian < portal.required_obsidian) {
					continue;
				}

				if (axe != 1) {
					continue;
				}

				if (sword != 0) {
					xoro = getRandomSequenceXoro(world_seed, blaze_str);
					for (i = 0; i < 2; ++i) {
						rods += xNextInt(&xoro, 2) + round(xNextFloat(&xoro) * (float)sword);
					}

					if (rods < required_rods) {
						continue;
					}
				} else {
					xoro = getRandomSequenceXoro(world_seed, blaze_str);
					for (i = 0; i < 2; ++i) {
						rods += xNextInt(&xoro, 2);
					}

					if (rods < required_rods) {
						continue;
					}
				}


				xoro = getRandomSequenceXoro(world_seed, bartering_str);
				for (i = 0; i < min(MAX_INGOTS, gold_ingots); ++i) {
				    BarteringOut out = nextBarteringLoot(&xoro);

				    if (out.id == 6) {
				    	pearls += out.amount;
				    }

				    if (out.id == 7) {
				    	string += out.amount;
				    }

				    if (out.id == 11) {
				    	nether_charges += out.amount;
				    }

				    if (out.id == 9) {
				    	obsidian += out.amount;
				    }

				    if (out.id == 5) {
				    	iron_nuggets += out.amount;
				    }

				    // bartered++;
				    // if (
				    // 	pearls >= required_pearls && 
				    // 	string >= 3 && 
				    // 	iron_ingots + iron_nuggets / 9 >= 26 && 
				    // 	obsidian + portal_obsidian - portal.required_obsidian >= 10 &&
				    // 	(flint_and_steel || (charges && flint) || (charges && nether_charges) || (charges >= 2))
				    // ) {
				    // 	break;
				    // }
			    }

				if (!(
					(flint_and_steel || (charges && flint) || (charges && nether_charges) || (charges >= 2))
				)) {
					continue;
				}

				if (iron_ingots + iron_nuggets / 9 < 26) {
					continue;
				}

				// if (gold_ingots < bartered) {
				// 	continue;
				// }

				if ((obsidian + portal_obsidian - portal.required_obsidian) < 10) {
					continue;
				}

			    if (pearls < required_pearls) {
			    	continue;
			    }

				if (string < 3) {
					continue;
				}

				// applySeed(&g, DIM_OVERWORLD, world_seed);
				spawn = getSpawn(&g);

				if (abs(spawn.x - portal_pos.x) > 48 || abs(spawn.z - portal_pos.z) > 48) {
					continue;
				}

			    printf("%" PRIi64 ",%d,%d\n", world_seed, rods, pearls);
			}
		} 
	}

	return 0;
}