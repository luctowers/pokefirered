#include "randomizer.h"

#include "random.h"
#include "daycare.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/party_menu.h"
#include "data/pokemon/level_up_learnsets.h"
#include "data/pokemon/tmhm_learnsets.h"
#include "data/pokemon/tutor_learnsets.h"

static u8 GetBitIndices(u32 word, u16* indices)
{
    u32 count = 0;
    u32 i;
    for (i = 0; i < 32; i++)
    {
        u32 mask = 1 << i;
        if (word & mask)
        {
            indices[count] = i; 
            count++;
        }
    }
    return count;
}

static u8 GetTmHmMoves(u16 species, u16* moves)
{
    u8 moveCount1 = GetBitIndices(sTMHMLearnsets[species][0], moves);
    u8 moveCount2 = GetBitIndices(sTMHMLearnsets[species][1], &moves[moveCount1]);
    u32 i;
    for (i = 0; i < moveCount1; i++) {
        moves[i] = sTMHMMoves[moves[i]];
    }
    for (i = moveCount1; i < moveCount2; i++) {
        moves[i] = sTMHMMoves[32+moves[i]];
    }
    return moveCount1 + moveCount2;
}

static u8 GetTutorMoves(u16 species, u16* moves)
{
    u8 moveCount = GetBitIndices(sTutorLearnsets[species], moves);
    u32 i;
    for (i = 0; i < moveCount; i++) {
        moves[i] = sTutorMoves[moves[i]];
    }
    return moveCount;
}

u16 RandomSpeciesMove(u16 species) {
    struct Pokemon pokemon;
    u16 moves[512];
    u16 moveCount = 0;
    moveCount += GetLevelUpMovesBySpecies(species, &moves[moveCount]);
    moveCount += GetTmHmMoves(species, &moves[moveCount]);
    moveCount += GetTutorMoves(species, &moves[moveCount]);
    moveCount += GetEggMoves(species, &moves[moveCount]);
    return moves[Random() % moveCount];
}

u16 RandomSpecies()
{
    u16 species = 1 + Random() % 386;
    if (species >= SPECIES_CELEBI)
    {
        species += 25;
    }
    return species;
}
