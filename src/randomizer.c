#include "randomizer.h"

#include "random.h"
#include "daycare.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/party_menu.h"
#include "data/pokemon/level_up_learnsets.h"
#include "data/pokemon/tmhm_learnsets.h"
#include "data/pokemon/tutor_learnsets.h"

static const u16 BLOCKED_TMHM_MOVES[] = {
    // COMMON TMS
    MOVE_PROTECT,
    MOVE_RETURN,
    MOVE_ATTRACT,
    MOVE_FRUSTRATION,
    MOVE_HIDDEN_POWER,
    MOVE_SECRET_POWER,
    MOVE_REST,
    MOVE_FACADE,
    MOVE_SUNNY_DAY,
    MOVE_RAIN_DANCE,
    MOVE_TOXIC,
    MOVE_HYPER_BEAM,
    MOVE_SANDSTORM,
    MOVE_HAIL,
    // HMS
    MOVE_FLASH,
    MOVE_FLY,
    MOVE_STRENGTH,
    MOVE_ROCK_SMASH,
    MOVE_CUT,
    MOVE_SURF,
    MOVE_WATERFALL,
    MOVE_DIVE
};

static const u16 BLOCKED_MOVES[] = {
    // EVASION
    MOVE_DOUBLE_TEAM,
    MOVE_MINIMIZE,
};

static const u16 ALLOWED_HELD_ITEMS[] = {
    ITEM_BERRY_JUICE,
    ITEM_CHERI_BERRY,
    ITEM_CHESTO_BERRY,
    ITEM_PECHA_BERRY,
    ITEM_RAWST_BERRY,
    ITEM_ASPEAR_BERRY,
    ITEM_LEPPA_BERRY,
    ITEM_ORAN_BERRY,
    ITEM_PERSIM_BERRY,
    ITEM_LUM_BERRY,
    ITEM_SITRUS_BERRY,
    ITEM_FIGY_BERRY,
    ITEM_WIKI_BERRY,
    ITEM_MAGO_BERRY,
    ITEM_AGUAV_BERRY,
    ITEM_IAPAPA_BERRY,
    ITEM_LIECHI_BERRY,
    ITEM_GANLON_BERRY,
    ITEM_SALAC_BERRY,
    ITEM_PETAYA_BERRY,
    ITEM_APICOT_BERRY,
    ITEM_LANSAT_BERRY,
    ITEM_STARF_BERRY,
    ITEM_BRIGHT_POWDER,
    ITEM_WHITE_HERB,
    ITEM_MACHO_BRACE,
    ITEM_EXP_SHARE,
    ITEM_QUICK_CLAW,
    // ITEM_SOOTHE_BELL,
    ITEM_MENTAL_HERB,
    ITEM_CHOICE_BAND,
    ITEM_KINGS_ROCK,
    ITEM_SILVER_POWDER,
    // ITEM_AMULET_COIN,
    // ITEM_CLEANSE_TAG,
    // ITEM_SOUL_DEW,
    // ITEM_DEEP_SEA_TOOTH,
    // ITEM_DEEP_SEA_SCALE,
    ITEM_SMOKE_BALL,
    ITEM_EVERSTONE,
    ITEM_FOCUS_BAND,
    ITEM_LUCKY_EGG,
    ITEM_SCOPE_LENS,
    ITEM_METAL_COAT,
    ITEM_LEFTOVERS,
    // ITEM_DRAGON_SCALE,
    ITEM_LIGHT_BALL,
    ITEM_SOFT_SAND,
    ITEM_HARD_STONE,
    ITEM_MIRACLE_SEED,
    ITEM_BLACK_GLASSES,
    ITEM_BLACK_BELT,
    ITEM_MAGNET,
    ITEM_MYSTIC_WATER,
    ITEM_SHARP_BEAK,
    ITEM_POISON_BARB,
    ITEM_NEVER_MELT_ICE,
    ITEM_SPELL_TAG,
    ITEM_TWISTED_SPOON,
    ITEM_CHARCOAL,
    ITEM_DRAGON_FANG,
    ITEM_SILK_SCARF,
    // ITEM_UP_GRADE,
    ITEM_SHELL_BELL,
    ITEM_SEA_INCENSE,
    ITEM_LAX_INCENSE,
    ITEM_LUCKY_PUNCH,
    ITEM_METAL_POWDER,
    ITEM_THICK_CLUB,
    ITEM_STICK
};

static u32 GetBitIndices(u32 word, u16 *indices)
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

static u32 GetTmHmMoves(u16 species, u16 *moves)
{
    u32 moveCount1 = GetBitIndices(sTMHMLearnsets[species][0], moves);
    u32 moveCount2 = GetBitIndices(sTMHMLearnsets[species][1], &moves[moveCount1]);
    u32 moveCountTotal = moveCount1 + moveCount2;
    u32 i;
    for (i = 0; i < moveCount1; i++)
    {
        moves[i] = sTMHMMoves[moves[i]];
    }
    for (i = moveCount1; i < moveCountTotal; i++)
    {
        moves[i] = sTMHMMoves[32+moves[i]];
    }
    return moveCountTotal;
}

// static u32 GetTutorMoves(u16 species, u16 *moves)
// {
//     u32 moveCount = GetBitIndices(sTutorLearnsets[species], moves);
//     u32 i;
//     for (i = 0; i < moveCount; i++)
//     {
//         moves[i] = sTutorMoves[moves[i]];
//     }
//     return moveCount;
// }

static u32 GetKnownMoves(struct BoxPokemon *boxMon, u16 *moves)
{
    u8 moveCount = 0;
    u32 i;
    for (i = 0; i < 4; i++)
    {
        u16 move = GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, NULL);
        if (move)
        {
            moves[moveCount] = move;
            moveCount++;
        }
    }
    return moveCount;
}

static void ShuffleMoves(u16 seed, u16 *moves, u32 moveCount)
{
    u32 i;
    SeedRng2(seed);
    for (i = moveCount-1; i > 0; i--)
    {
        u32 j = Random2() % (i+1);
        u16 temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
    }
}

static void EnsureAttackingMoves(u16 *moves, u32 moveCount)
{
    u32 nonAttackingMoveIndices[4];
    u32 nonAttackingMoveCount = 0;
    u32 i;
    if (moveCount < 5) {
        return;
    }
    for (i = 0; i < 4; i++)
    {
        if (gBattleMoves[moves[i]].power == 0) {
            nonAttackingMoveIndices[nonAttackingMoveCount] = i;
            nonAttackingMoveCount++;
        }
    }
    i = moveCount;
    while (nonAttackingMoveCount > 2 && i >= 4) {
        if (gBattleMoves[moves[i]].power != 0) {
            u16 temp;
            nonAttackingMoveCount--;
            temp = moves[i];
            moves[i] = moves[nonAttackingMoveIndices[nonAttackingMoveCount]];
            moves[nonAttackingMoveIndices[nonAttackingMoveCount]] = temp;
        }
        i--;
    }
}

static u32 FilterMoves(bool32 (*filter)(u16,void*), void *filterArgs, u16 *moves, u32 moveCount)
{
    u32 removeCount = 0;
    u32 writeIndex = 0;
    u32 readIndex;
    for (readIndex = 0; readIndex < moveCount; readIndex++)
    {
        if (!filter(moves[readIndex], filterArgs))
        {
            removeCount++;
        }
        else 
        {
            moves[writeIndex] = moves[readIndex];
            writeIndex++;
        }
    }
    return removeCount;
}

struct RemoveMoveFilterArgs {
    const u16 *removes;
    u32 removeCount;
};

static bool32 RemoveMovesFilter(u16 move, void *args)
{
    struct RemoveMoveFilterArgs *typedArgs = (struct RemoveMoveFilterArgs*) args;
    u32 i;
    for (i = 0; i < typedArgs->removeCount; i++)
    {
        if (move == typedArgs->removes[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

static u32 RemoveMoves(const u16 *removes, u8 removeCount, u16 *moves, u8 moveCount)
{
    struct RemoveMoveFilterArgs filterArgs = { removes, removeCount };
    return FilterMoves(RemoveMovesFilter, &filterArgs, moves, moveCount);
}

u16 RandomMove(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL);
    u16 metLevel = GetBoxMonData(boxMon, MON_DATA_MET_LEVEL, NULL);
    u16 level = GetLevelFromBoxMonExp(boxMon);
    u16 knownMoves[4];
    u32 knownMoveCount = GetKnownMoves(boxMon, knownMoves);
    u16 moves[256];
    u32 moveCount = 0;
    moveCount += GetTmHmMoves(species, &moves[moveCount]);
    moveCount -= RemoveMoves(BLOCKED_TMHM_MOVES, NELEMS(BLOCKED_TMHM_MOVES), moves, moveCount);
    moveCount += GetEggMoves(boxMon, &moves[moveCount]);
    moveCount += GetLevelUpMovesBySpecies(species, &moves[moveCount]);
    // moveCount += GetTutorMoves(species, &moves[moveCount]);
    moveCount -= RemoveMoves(BLOCKED_MOVES, NELEMS(BLOCKED_MOVES), moves, moveCount);
    ShuffleMoves(boxMon->personality, moves, moveCount);
    EnsureAttackingMoves(moves, moveCount);
    if (knownMoveCount < 4 && moveCount > knownMoveCount) {
        return moves[knownMoveCount];
    } else if (moveCount) {
        u16 move = moves[(level-metLevel+3)%moveCount];
        bool32 moveAlreadyKnown = FALSE;
        u32 i;
        for (i = 0; i < knownMoveCount; i++) {
            if (knownMoves[i] == move) {
                moveAlreadyKnown = TRUE;
                break;
            }
        }
        if (!moveAlreadyKnown) {
            return move;
        } else {
            moveCount -= RemoveMoves(knownMoves, knownMoveCount, moves, moveCount);
            if (moveCount) {
                return moves[Random() % moveCount];
            }
        }
    }
    return MOVE_NONE;
}

u16 RandomSpecies()
{
    return RandomSpeciesFromSeed(Random());
}

u16 RandomSpeciesFromSeed(u16 seed)
{
    u16 species;
    SeedRng2(seed);
    do {
        species = 1 + Random2() % 386;
        if (species > SPECIES_CELEBI)
        {
            species += 25;
        }
    } while (gSmogonTiers[species] == TIER_UBER);
    return species;
}

u16 RandomHeldItem()
{
    return ALLOWED_HELD_ITEMS[Random() % NELEMS(ALLOWED_HELD_ITEMS)];
}
