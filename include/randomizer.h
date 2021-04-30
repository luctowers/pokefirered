#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

#include "global.h"

u16 RandomSpecies();
u16 RandomSpeciesFromSeed(u16 seed);
u16 RandomMove(struct BoxPokemon *boxMon);

#endif // GUARD_RANDOMIZER_H
