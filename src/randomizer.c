#include "randomizer.h"

#include "random.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "data/pokemon/level_up_learnsets.h"
#include "data/pokemon/tmhm_learnsets.h"

static u32 countSetBits(u32 word)
{
    u32 result = 0;
    u32 mask;
    u32 i;
    for (i = 0; i < 32; i++)
    {
        mask = 1 << i;
        if (word & mask)
        {
            result++;
        }
    }
    return result;
}

static u32 findNthSetBit(u32 word, u16 n)
{
    u32 count = 0;
    u32 mask;
    u32 i;
    for (i = 0; i < 32; i++)
    {
        mask = 1 << i;
        if (word & mask)
        {
            if (count == n) {
                return i;
            }
            count++;
        }
    }
    return 32;
}

u16 randomSpecies()
{
    u16 species = 1 + Random() % 386;
    if (species >= SPECIES_CELEBI)
    {
        species += 25;
    }
    return species;
}

u16 randomSpeciesMove(u16 species) {
    u16 moveIndex;
    u32 levelUpMoveCount = 0;
    u32 tmhmMoveCount = countSetBits(sTMHMLearnsets[species][0]) + countSetBits(sTMHMLearnsets[species][1]);
    while (gLevelUpLearnsets[species][levelUpMoveCount] != LEVEL_UP_END)
    {
        levelUpMoveCount++;
    }
    moveIndex = Random() % (levelUpMoveCount + tmhmMoveCount);
    if (moveIndex < levelUpMoveCount)
    {
        return gLevelUpLearnsets[species][moveIndex] & 0x1FF;
    } else
    {
        u32 tmhmIndex;
        moveIndex -= levelUpMoveCount;
        if (moveIndex < tmhmMoveCount)
        {
            tmhmIndex = findNthSetBit(sTMHMLearnsets[species][0], moveIndex);
            if (tmhmIndex == 32) // tm not found is first 32-bit word
            {
                moveIndex -= countSetBits(sTMHMLearnsets[species][0]);
                tmhmIndex += findNthSetBit(sTMHMLearnsets[species][1], moveIndex);
            }
            return sTMHMMoves[tmhmIndex];
        }
    }
}
