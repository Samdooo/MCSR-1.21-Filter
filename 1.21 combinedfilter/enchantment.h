#pragma once

#include "utilities.h"

#ifndef ENCHANTMENT_H
#define ENCHANTMENT_H

typedef struct {
    int id;
    int max_level;
} Enchantment;

typedef struct {
    int id;
    int level;
    int weight;
} EnchantmentHelper;

typedef struct {
    int id;
    int levels;
    int weight;
    Iterval thresholds[5];
} ETableEnchantment;

#ifdef __cplusplus
extern "C"
{
#endif

void print_enchantment(const int id, const int level, const int new_line);

#ifdef __cplusplus
}
#endif
// int get_available_enchantments(const int level, const EquipmentType type, EnchantmentHelper* available);
// int get_random_enchantment(uint64_t* rand, const EnchantmentHelper* list, const int length);
// int remove_conflicting_enchantments(const int to_remove, EnchantmentHelper* list, const int old_length);
// int enchant_with_levels_full(uint64_t* rand, int level, const int ench_val, const EquipmentType equipment_type, EnchantmentHelper* list);

#endif