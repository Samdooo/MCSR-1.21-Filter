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
    ofstream ofile("trades.txt", ios::app);
    string line;
    while (getline(ifile, line)){
        stringstream sline(line);
        uint64_t seed;
        sline >> seed;
        char junk;
        int numEyes;
        sline >> junk >> numEyes >> junk >> numEyes;
        
        Xoroshiro xoro = getRandomSequenceXoro(seed, bartering_str);
        
        stringstream snewline;
        snewline << line;
        snewline << " ; ";
        
        int numPearls = 0;
        for (int i = 0; i < MAX_INGOTS; ++i) {
            BarteringOut out = nextBarteringLoot(&xoro);

            if (names.count(out.id)){
                snewline << out.amount << " " << names[out.id] << (i + 1 < MAX_INGOTS ? ", " : "");
            }
            else {
                snewline << "junk" << (i + 1 < MAX_INGOTS ? ", " : "");
            }
            
            if (out.id == 6) {
                numPearls += out.amount;
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
        if (numPearls >= (12 - numEyes) + 1){
            ofile << snewline.str() << "\n";
        }
    }
    
	return 0;
}