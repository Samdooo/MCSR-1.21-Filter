#include <bits/stdc++.h>
#include <fstream>

using namespace std;

extern "C" {
#include <omp.h>
#include <inttypes.h>
#include "cubiomes.h"
#include "bastion.h"
#include "ruined_portal.h"
#include "MiLTSU.h"
}

#define MAX_INGOTS 9

int main() {
    
    char* bartering_str = "minecraft:gameplay/piglin_bartering";
    
    unordered_map<int, string> names = {
        { 6, "pearls" },
        { 7, "string" },
        { 11, "fire charge" },
        { 9, "obsidian" },
        { 5, "iron nuggets" }
    };
    
    ifstream ifile("result.txt");
    ofstream ofile("trades.txt");
    string line;
    while (getline(ifile, line)){
        stringstream sline(line);
        uint64_t seed;
        sline >> seed;
        char junk;
        sline >> junk;
        int numEyes;
        sline >> numEyes;
        
        Xoroshiro xoro = getRandomSequenceXoro(seed, bartering_str);
        
        ofile << line << " ; ";
        
        for (int i = 0; i < MAX_INGOTS; ++i) {
            BarteringOut out = nextBarteringLoot(&xoro);

            if (names.count(out.id)){
                ofile << out.amount << " " << names[out.id] << (i + 1 < MAX_INGOTS ? ", " : "");
            }
            else {
                ofile << "junk" << (i + 1 < MAX_INGOTS ? ", " : "");
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
        
        ofile << "\n";
        
    }
    
	return 0;
}