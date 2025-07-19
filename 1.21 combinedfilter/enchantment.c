#include "enchantment.h"
#include "cubiomes/rng.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>

// const Enchantment ENCHANTMENTS[] = {
//     { 0, 4 }, // protection
//     { 1, 4 }, // fire_protection
//     { 2, 4 }, // feather_falling
//     { 3, 4 }, // blast_protection
//     { 4, 4 }, // projectile_protection
//     { 5, 3 }, // respiration
//     { 6, 1 }, // aqua_affinity
//     { 7, 3 }, // thorns
//     { 8, 3 }, // depth_strider
//     { 9, 2 }, // frost_walker
//     { 10, 1 }, // binding_curse
//     { 11, 3 }, // soul_speed
//     { 12, 3 }, // swift_sneak
//     { 13, 5 }, // sharpness
//     { 14, 5 }, // smite
//     { 15, 5 }, // bane_of_arthropods
//     { 16, 2 }, // knockback
//     { 17, 2 }, // fire_aspect
//     { 18, 3 }, // looting
//     { 19, 3 }, // sweeping
//     { 20, 5 }, // efficiency
//     { 21, 1 }, // silk_touch
//     { 22, 3 }, // unbreaking
//     { 23, 3 }, // fortune
//     { 24, 5 }, // power
//     { 25, 2 }, // punch
//     { 26, 1 }, // flame
//     { 27, 1 }, // infinity
//     { 28, 3 }, // luck_of_the_sea
//     { 29, 3 }, // lure
//     { 30, 3 }, // loyalty
//     { 31, 5 }, // impaling
//     { 32, 3 }, // riptide
//     { 33, 1 }, // channeling
//     { 34, 1 }, // multishot
//     { 35, 3 }, // quick_charge
//     { 36, 4 }, // piercing
//     { 37, 1 }, // mending
//     { 38, 1 }, // vanishing_curse
// };
// const int ENCHANTMENTS_NUMBER = 39;


const ETableEnchantment ETABLE_ENCHANTMENTS[] = {
    { .id = 0, .levels = 4,  .weight = 10, .thresholds = {{ 1, 12 }, { 12, 23 }, { 23, 34 }, { 34, 45 }}},                // Protection
    { .id = 1, .levels = 4,  .weight = 5, .thresholds = {{ 10, 18 }, { 18, 26 }, { 26, 34 }, { 34, 42 }}},                // Fire Protection
    { .id = 2, .levels = 4,  .weight = 5, .thresholds = {{ 5, 11 }, { 11, 17 }, { 17, 23 }, { 23, 29 }}},                 // Feather Falling
    { .id = 3, .levels = 4,  .weight = 2, .thresholds = {{ 5, 13 }, { 13, 21 }, { 21, 29 }, { 29, 37 }}},                 // Blast Protection
    { .id = 4, .levels = 4,  .weight = 5, .thresholds = {{ 3, 9 }, { 9, 15 }, { 15, 21 }, { 21, 27 }}},                   // Projectile Protection
    { .id = 5, .levels = 3,  .weight = 2, .thresholds = {{ 10, 40 }, { 20, 50 }, { 30, 60 }}},                            // Respiration
    { .id = 6, .levels = 1,  .weight = 2, .thresholds = {{ 1, 41 }}},                                                     // Aqua Affinity
    { .id = 7, .levels = 3,  .weight = 1, .thresholds = {{ 10, 60 }, { 30, 80 }, { 50, 100 }}},                           // Thorns
    { .id = 8, .levels = 3,  .weight = 2, .thresholds = {{ 10, 25 }, { 20, 35 }, { 30, 45 }}},                            // Depth Strider
    { .id = 9, .levels = 2,  .weight = 2, .thresholds = {{ 10, 25 }, { 20, 35 }}},                                        // Frost Walker
    { .id = 10, .levels = 1, .weight = 1,  .thresholds = {{ 25, 50 }}},                                                   // Curse of Binding
    { .id = 11, .levels = 5, .weight = 10,  .thresholds = {{ 1, 21 }, { 12, 32 }, { 23, 43 }, { 34, 54 }, { 45, 65 }}},   // Sharpness
    { .id = 12, .levels = 5, .weight = 5,  .thresholds = {{ 5, 25 }, { 13, 33 }, { 21, 41 }, { 29, 49 }, { 37, 57 }}},    // Smite
    { .id = 13, .levels = 5, .weight = 5,  .thresholds = {{ 5, 25 }, { 13, 33 }, { 21, 41 }, { 29, 49 }, { 37, 57 }}},    // Bane of Arthropods
    { .id = 14, .levels = 2, .weight = 5,  .thresholds = {{ 5, 55 }, { 25, 75 }}},                                        // Knockback
    { .id = 15, .levels = 2, .weight = 2,  .thresholds = {{ 10, 60 }, { 30, 80 }}},                                       // Fire Aspect
    { .id = 16, .levels = 3, .weight = 2, .thresholds = {{ 15, 65 }, { 24, 74 }, { 33, 83 }}},                            // Looting
    { .id = 17, .levels = 3, .weight = 2,  .thresholds = {{ 5, 20 }, { 14, 29 }, { 23, 38 }}},                            // Sweeping Edge
    { .id = 18, .levels = 5, .weight = 10, .thresholds = {{ 1, 51 }, { 11, 61 }, { 21, 71 }, { 31, 81 }, { 41, 91 }}},    // Efficiency
    { .id = 19, .levels = 1, .weight = 1,  .thresholds = {{ 15, 65 }}},                                                   // Silk Touch
    { .id = 20, .levels = 3, .weight = 5,  .thresholds = {{ 5, 55 }, { 13, 63 }, { 21, 71 }}},                            // Unbreaking
    { .id = 21, .levels = 3, .weight = 2,  .thresholds = {{ 15, 65 }, { 24, 74 }, { 33, 83 }}},                           // Fortune
    { .id = 35, .levels = 1, .weight = 2,  .thresholds = {{ 25, 75 }}},                                                   // Mending
    { .id = 36, .levels = 1, .weight = 1,  .thresholds = {{ 25, 50 }}},                                                   // Curse of Vanishing
};
const int ETABLE_ENCHANTMENTS_NUMBER = 24;
// const int BOOTS_ENCHANTMENTS[]              = {0, 1, 2, 3, 4, 7, 8, 9, 10, 20, 35, 36};
const Enchantment BOOK_ENCHANTMENTS[] = {
    { 0, 4 }, // protection
    { 1, 4 }, // fire_protection
    { 2, 4 }, // feather_falling
    { 3, 4 }, // blast_protection
    { 4, 4 }, // projectile_protection
    { 5, 3 }, // respiration
    { 6, 1 }, // aqua_affinity
    { 7, 3 }, // thorns
    { 8, 3 }, // depth_strider
    { 9, 2 }, // frost_walker
    { 10, 1 }, // binding_curse
    { 11, 5 }, // sharpness
    { 12, 5 }, // smite
    { 13, 5 }, // bane_of_arthropods
    { 14, 2 }, // knockback
    { 15, 2 }, // fire_aspect
    { 16, 3 }, // looting
    { 17, 3 }, // sweeping
    { 18, 5 }, // efficiency
    { 19, 1 }, // silk_touch
    { 20, 3 }, // unbreaking
    { 21, 3 }, // fortune
    { 22, 5 }, // power
    { 23, 2 }, // punch
    { 24, 1 }, // flame
    { 25, 1 }, // infinity
    { 26, 3 }, // luck_of_the_sea
    { 27, 3 }, // lure
    { 28, 3 }, // loyalty
    { 29, 5 }, // impaling
    { 30, 3 }, // riptide
    { 31, 1 }, // channeling
    { 32, 1 }, // multishot
    { 33, 3 }, // quick_charge
    { 34, 4 }, // piercing
    { 35, 1 }, // mending
    { 36, 1 }, // vanishing_curse
};

// const int AXE_ENCHANTMENTS[]                = {11, 12, 13, 18, 19, 20, 21, 35, 36};
// const int SWORD_ENCHANTMENTS[]              = {11, 12, 13, 14, 15, 16, 17, 20, 35, 36};
// const int TOOL_ENCHANTMENTS[]               = {18, 19, 20, 21, 35, 36};

const int BOOK_ENCHANTMENTS_NUMBER = 37;

const int TOOL_ENCHANTMENTS[]                  = {18, 19, 20, 21, 36, 35};
const int AXE_ENCHANTMENTS[]                    = {11, 12, 13, 18, 19, 20, 21, 36, 35};
const int SWORD_ENCHANTMENTS[]               = {11, 12, 13, 14, 15, 16, 17, 20, 36, 35};
const int CROSSBOW_ENCHANTMENTS[]        = { 20, 32, 33, 34, 36, 35};
// const int BOOTS_ENCHANTMENTS[]                = {0, 1, 2, 3, 4, 7, 8, 9, 10, 20, 35, 36};
// const int LEGGINGS_ENCHANTMENTS[]           = {0, 1, 3, 4, 7, 10, 20, 35, 36};
// const int CHESTPLATE_ENCHANTMENTS[]       = {0, 1, 3, 4, 7, 10, 20, 35, 36};
// const int HELMET_ENCHANTMENTS[]              = {0, 1, 3, 4, 5, 6, 7, 10, 20, 35, 36};
const int BOOTS_ENCHANTMENTS[]                = {0, 1, 2, 3, 4, 7, 8, 20, 10, 36, 9, 35};
const int LEGGINGS_ENCHANTMENTS[]           = {0, 1, 3, 4, 7, 20, 10, 36, 35};
const int CHESTPLATE_ENCHANTMENTS[]       = {0, 1, 3, 4, 7, 20, 10, 36, 35};
const int HELMET_ENCHANTMENTS[]              = {0, 1, 3, 4, 5, 6, 7, 20, 10, 36, 35};

// buty
// 0, 1, 2, 3, 4, 7, 8, 9, 10, 20, 35, 36
// 9 -> 20
// 20 -> 36
// 35 -> 9
// 36 -> 35

// spodnie
// 0, 1, 3, 4, 7, 10, 20, 35, 36
// 10 -> 20
// 20 -> 10
// 35 -> 36
// 36 -> 35

// napierśnik
// 0, 1, 3, 4, 7, 10, 20, 35, 36
// 10 -> 20
// 20 -> 10
// 35 -> 36
// 36 -> 35

// czapka
// 0, 1, 3, 4, 5, 6, 7, 10, 20, 35, 36
// 10 -> 20
// 20 -> 10
// 35 -> 36
// 36 -> 35

const int TOOL_ENCHANTMENTS_NUMBER          = 6;
const int CROSSBOW_ENCHANTMENTS_NUMBER          = 6;
const int AXE_ENCHANTMENTS_NUMBER           = 9;
const int SWORD_ENCHANTMENTS_NUMBER         = 10;
const int BOOTS_ENCHANTMENTS_NUMBER         = 12;
const int LEGGINGS_ENCHANTMENTS_NUMBER      = 9;
const int CHESTPLATE_ENCHANTMENTS_NUMBER    = 9;
const int HELMET_ENCHANTMENTS_NUMBER        = 11;

void print_enchantment(const int id, const int level, const int new_line) {
    switch (id) {
        case 0: printf("protection %d", level); break;
        case 1: printf("fire_protection %d", level); break;
        case 2: printf("feather_falling %d", level); break;
        case 3: printf("blast_protection %d", level); break;
        case 4: printf("projectile_protection %d", level); break;
        case 5: printf("respiration %d", level); break;
        case 6: printf("aqua_affinity %d", level); break;
        case 7: printf("thorns %d", level); break;
        case 8: printf("depth_strider %d", level); break;
        case 9: printf("frost_walker %d", level); break;
        case 10: printf("binding_curse %d", level); break;
        case 11: printf("sharpness %d", level); break;
        case 12: printf("smite %d", level); break;
        case 13: printf("bane_of_arthropods %d", level); break;
        case 14: printf("knockback %d", level); break;
        case 15: printf("fire_aspect %d", level); break;
        case 16: printf("looting %d", level); break;
        case 17: printf("sweeping %d", level); break;
        case 18: printf("efficiency %d", level); break;
        case 19: printf("silk_touch %d", level); break;
        case 20: printf("unbreaking %d", level); break;
        case 21: printf("fortune %d", level); break;
        case 22: printf("power %d", level); break;
        case 23: printf("punch %d", level); break;
        case 24: printf("flame %d", level); break;
        case 25: printf("infinity %d", level); break;
        case 26: printf("luck_of_the_sea %d", level); break;
        case 27: printf("lure %d", level); break;
        case 28: printf("loyalty %d", level); break;
        case 29: printf("impaling %d", level); break;
        case 30: printf("riptide %d", level); break;
        case 31: printf("channeling %d", level); break;
        case 32: printf("multishot %d", level); break;
        case 33: printf("quick_charge %d", level); break;
        case 34: printf("piercing %d", level); break;
        case 35: printf("mending %d", level); break;
        case 36: printf("vanishing_curse %d", level); break;
    }
    if (new_line) printf("\n");
}

// int get_available_enchantments(const int level, const EquipmentType type, EnchantmentHelper* available) {
//     int n;
//     int list[30];

//     switch (type) {
//         case SWORD: {
//             n = SWORD_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &SWORD_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }

//         case PICKAXE: {
//             n = TOOL_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &TOOL_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }

//         case SHOVEL: {
//             n = TOOL_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &TOOL_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }
        
//         case HELMET: {
//             n = HELMET_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &HELMET_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }
        
//         case CHESTPLATE: {
//             n = CHESTPLATE_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &CHESTPLATE_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }
        
//         case LEGGINGS: {
//             n = LEGGINGS_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &LEGGINGS_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }
        
//         case BOOTS: {
//             n = BOOTS_ENCHANTMENTS_NUMBER;
//             memcpy(&list, &BOOTS_ENCHANTMENTS, n * sizeof(int));
//             break;
//         }
        
//         default: return 0;
//     }
//     // printf("zaklecie:\n");
//     // for (int i = 0; i < n; ++i) {
//     //     printf("%d\n", list[i]);
//     // }

//     // printf("Poziom: %d\n", level);
//     int offset = 0;
//     for (int i = 0; i < n; ++i) {
//         int id = list[i];

//         if (id == 7 && type != CHESTPLATE) continue; // ONLY CHESTPLATES CAN BE ENCHANTED WITH THORNS!!

//         for (int j = 0; j < ETABLE_ENCHANTMENTS_NUMBER; ++j) {
//             if (id == ETABLE_ENCHANTMENTS[j].id) {
//                 for (int k = ETABLE_ENCHANTMENTS[j].levels - 1; k >= 0; --k) {
//                     if (ETABLE_ENCHANTMENTS[j].thresholds[k].min <= level && level <= ETABLE_ENCHANTMENTS[j].thresholds[k].max) {
//                         // printf("%d, %d, %d, %d\n", offset, id, k+1, ETABLE_ENCHANTMENTS[j].weight);
//                         available[offset].id = id;
//                         available[offset].level = k;
//                         available[offset].weight = ETABLE_ENCHANTMENTS[j].weight;
//                         ++offset;
//                         break;
//                     }
//                 }

//                 break;
//             }
//         }
//     }

//     return offset;
// }

// int get_random_enchantment(uint64_t* rand, const EnchantmentHelper* list, const int length) {
//     int total_weight = 0;
//     for (int i = 0; i < length; ++i) {
//         total_weight += list[i].weight;
//     }

//     int roll = nextInt(rand, total_weight);
//     // printf("Roll: %d, %d\n", roll, total_weight);

//     for (int i = 0; i < length; ++i) {
//         roll -= list[i].weight;

//         if (roll < 0) {
//             return i;
//         }
//     }
// }

// int remove_conflicting_enchantments(const int to_remove, EnchantmentHelper* list, const int old_length) {
//     int new_length = 0;
//     EnchantmentHelper new_list[30];
//     for (int i = 0; i < old_length; ++i) {
//         int id = list[i].id;
//         // Remove self.
//         if (id == to_remove) continue;
        
//         // Remove enchantments conflicting with Sharpness.
//         if (to_remove == 11 && (id == 12 || id == 13)) continue;
//         // Remove enchantments conflicting with Smite.
//         if (to_remove == 12 && (id == 11 || id == 13)) continue;
//         // Remove enchantments conflicting with Bane of Arthropods.
//         if (to_remove == 13 && (id == 11 || id == 12)) continue;
        
//         // Remove enchantments conflicting with Protection.
//         if (to_remove == 0 && (id == 1 || id == 3 || id == 4)) continue;
//         // Remove enchantments conflicting with Fire Protection.
//         if (to_remove == 1 && (id == 0 || id == 3 || id == 4)) continue;
//         // Remove enchantments conflicting with Blast Protection.
//         if (to_remove == 3 && (id == 0 || id == 1 || id == 4)) continue;
//         // Remove enchantments conflicting with Projectile Protection.
//         if (to_remove == 4 && (id == 0 || id == 1 || id == 3)) continue;

//         // Remove enchantments conflicting with Fortune.
//         if (to_remove == 21 && id == 19) continue;
//         // Remove enchantments conflicting with Silk Touch.
//         if (to_remove == 19 && id == 21) continue;

//         // Remove enchantments conflicting with Depth Strider.
//         if (to_remove == 8 && id == 9) continue;
//         // Remove enchantments conflicting with Frost Walker.
//         if (to_remove == 9 && id == 8) continue;
        
//         new_list[new_length].id = id;
//         new_list[new_length].level = list[i].level;
//         new_list[new_length].weight = list[i].weight;
//         ++new_length;
//     }

//     for (int i = 0; i < new_length; ++i) {
//         list[i].id = new_list[i].id;
//         list[i].level = new_list[i].level;
//         list[i].weight = new_list[i].weight;
//     }

//     return new_length;
// }

// int enchant_with_levels_full(uint64_t* rand, int level, const int ench_val, const EquipmentType equipment_type, EnchantmentHelper* list) {
//     int enchantments = 0;
//     EnchantmentHelper available_enchantments[30] = { -1 };

//     level += 1 + nextInt(rand, ench_val / 4 + 1) + nextInt(rand, ench_val / 4 + 1);
//     float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0f) * 0.15f;
//     level = clamp((int)roundf((float)level + (float)level * amplifier), 1, INT_MAX);
    
//     int length = get_available_enchantments(level, equipment_type, available_enchantments);
//     if (length != 0) {
//         int enchantment_index = get_random_enchantment(rand, available_enchantments, length);
//         list[enchantments].id = available_enchantments[enchantment_index].id;
//         list[enchantments].level = available_enchantments[enchantment_index].level + 1;
//         ++enchantments;
//         length = remove_conflicting_enchantments(available_enchantments[enchantment_index].id, available_enchantments, length);
//         // printf("Po usunueciu:\n");
//         // for (int i = 0; i < length; ++i) {
//         //     printf("%d\n", available_enchantments[i].id);
//         // }

//         while (nextInt(rand, 50) <= level) {
//             if (length == 0) break;
//             int enchantment_index = get_random_enchantment(rand, available_enchantments, length);
//             list[enchantments].id = available_enchantments[enchantment_index].id;
//             list[enchantments].level = available_enchantments[enchantment_index].level + 1;
//             ++enchantments;
//             length = remove_conflicting_enchantments(available_enchantments[enchantment_index].id, available_enchantments, length);

//             level /= 2;
//         }
//     }

//     return enchantments;
// }