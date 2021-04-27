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
    // HMS
    MOVE_FLASH,
    MOVE_FLY,
    MOVE_STRENGTH,
    MOVE_ROCK_SMASH,
    MOVE_CUT,
    MOVE_SURF,
    MOVE_WATERFALL
};

static const u16 BLOCKED_MOVES[] = {
    // EVASION
    MOVE_DOUBLE_TEAM,
    MOVE_MINIMIZE,
};

static u8 GetBitIndices(u32 word, u16 *indices)
{
    u32 count = 0;
    u32 i;
    for (i = 0; i < 32; i++)
    {
        u32 mask = 1 << i;
        if (word & mask) {
            indices[count] = i;
            count++;
        }
    }
    return count;
}

static u8 GetTmHmMoves(u16 species, u16 *moves)
{
    u8 moveCount1 = GetBitIndices(sTMHMLearnsets[species][0], moves);
    u8 moveCount2 = GetBitIndices(sTMHMLearnsets[species][1], &moves[moveCount1]);
    u8 moveCountTotal = moveCount1 + moveCount2;
    u32 i;
    for (i = 0; i < moveCount1; i++) {
        moves[i] = sTMHMMoves[moves[i]];
    }
    for (i = moveCount1; i < moveCountTotal; i++) {
        moves[i] = sTMHMMoves[32+moves[i]];
    }
    return moveCountTotal;
}

// static u8 GetTutorMoves(u16 species, u16 *moves)
// {
//     u8 moveCount = GetBitIndices(sTutorLearnsets[species], moves);
//     u32 i;
//     for (i = 0; i < moveCount; i++) {
//         moves[i] = sTutorMoves[moves[i]];
//     }
//     return moveCount;
// }

static u16 RemoveMoves(const u16 *bannedMoves, u16 bannedMoveCount, u16 *moves, u16 moveCount)
{
    u32 removeCount = 0;
    u32 writeIndex = 0;
    u32 readIndex;
    for (readIndex = 0; readIndex < moveCount; readIndex++)
    {
        bool32 banned = FALSE;
        u32 banIndex;
        for (banIndex = 0; banIndex < bannedMoveCount; banIndex++) {
            if (moves[readIndex] == bannedMoves[banIndex]) {
                banned = TRUE;
                break;
            }
        }
        if (banned) {
            removeCount++;
        } else {
            moves[writeIndex] = moves[readIndex];
            writeIndex++;
        }
    }
    return removeCount;
}

static u8 GetKnownMoves(struct Pokemon *mon, u16 *moves) {
    u8 moveCount = 0;
    u32 i;
    for (i = 0; i < 4; i++) {
        u16 move = GetBoxMonData(&mon->box, MON_DATA_MOVE1 + i, NULL);
        if (move) {
            moves[moveCount] = move;
            moveCount++;
        }
    }
    return moveCount;
}

u16 RandomSpeciesMove(struct Pokemon *mon) {
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u16 knownMoves[4];
    u16 knownMoveCount = GetKnownMoves(mon, knownMoves);
    u16 moves[512];
    u16 moveCount = 0;
    moveCount += GetTmHmMoves(species, &moves[moveCount]);
    moveCount -= RemoveMoves(BLOCKED_TMHM_MOVES, NELEMS(BLOCKED_TMHM_MOVES), moves, moveCount);
    moveCount += GetEggMoves(mon, &moves[moveCount]);
    moveCount += GetLevelUpMovesBySpecies(species, &moves[moveCount]);
    // moveCount += GetTutorMoves(species, &moves[moveCount]);
    moveCount -= RemoveMoves(knownMoves, knownMoveCount, moves, moveCount);
    moveCount -= RemoveMoves(BLOCKED_MOVES, NELEMS(BLOCKED_MOVES), moves, moveCount);
    if (moveCount) {
        return moves[Random() % moveCount];
    } else {
        return MOVE_NONE;
    }
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
