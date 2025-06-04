#ifndef SPECTRAL_H
#define SPECTRAL_H

#include <stdint.h>

typedef enum {
  SPECTRAL_FAMILIAR,
  SPECTRAL_GRIM,
  SPECTRAL_INCANTATION,
  SPECTRAL_TALISMAN,
  SPECTRAL_AURA,
  SPECTRAL_WRAITH,
  SPECTRAL_SIGIL,
  SPECTRAL_OUIJA,
  SPECTRAL_ECTOPLASM,
  SPECTRAL_IMMOLATE,
  SPECTRAL_ANKH,
  SPECTRAL_DEJA_VU,
  SPECTRAL_HEX,
  SPECTRAL_TRANCE,
  SPECTRAL_MEDIUM,
  SPECTRAL_CRYPTID,
  SPECTRAL_SOUL,
  SPECTRAL_BLACK_HOLE
} Spectral;

const char *get_spectral_card_name(Spectral spectral);
const char *get_spectral_card_description(Spectral spectral);

uint8_t get_spectral_max_selected(Spectral spectral);
uint8_t use_spectral_card(Spectral spectral);

#endif
