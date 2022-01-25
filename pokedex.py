import os
"""
Yes, this code is bad
I wrote it at like 2am
i was tired
leave me alone
"""

SPECIES_H = "C:/Users/Kai/Documents/GitHub/pokeemerald-2/include/constants/species.h"
NUM_NATIONAL_DEX = 901
NUM_HOENN_DEX = 212


if __name__ == "__main__":
    with open(SPECIES_H, "r") as file:
        lines = file.readlines()

        for i in range(0, 1349):
            lines.pop(0)
        try:
            for i in range(0, 228):
                lines.pop(1121)
        except IndexError:
            pass

        lines.remove("#define NATIONAL_DEX_COUNT NATIONAL_DEX_CALYREX\n")
        lines.remove("#define POKEMON_SLOTS_NUMBER (NATIONAL_DEX_COUNT + 1)\n")

        print(lines)

        noHashDefine = []

        for line in lines:
            data = line.split(" ")
            if len(data) < 3: # or "hoenn" in data[1].lower():
                continue
            print(data)
            noHashDefine.append(data[1])

        with open("pokedex.txt", "w") as pokedex:
            pokedex.write("#ifndef GUARD_CONSTANTS_POKEDEX_H\n\
#define GUARD_CONSTANTS_POKEDEX_H\n\
// National Pokedex order\n\
enum {\n")

            for i in range(0, NUM_NATIONAL_DEX):
                pokedex.write(f"    "
                              f"{noHashDefine[i]},\n")

            pokedex.write("};\n\
#define NATIONAL_DEX_COUNT NATIONAL_DEX_CALYREX\n\
#define POKEMON_SLOTS_NUMBER (NATIONAL_DEX_COUNT + 1)\n\
// Hoenn Pokedex order\n\
enum {\n")
            for i in range(0, NUM_HOENN_DEX):
                if noHashDefine[i+NUM_NATIONAL_DEX] == "Hoenn":
                    continue
                pokedex.write(f"    {noHashDefine[i+NUM_NATIONAL_DEX]},\n")

            pokedex.write("};\n")


