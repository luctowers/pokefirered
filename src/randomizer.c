#include "randomizer.h"

u16 randomSpecies() {
  u16 species = 1 + Random() % 386;
  if (species >= SPECIES_CELEBI) {
    species += 25;
  }
  return species;
}
