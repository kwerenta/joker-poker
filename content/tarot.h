#ifndef TAROT_H
#define TAROT_H

#include <stdint.h>

typedef enum {
  TAROT_FOOL,
  TAROT_MAGICIAN,
  TAROT_HIGH_PRIESTESS,
  TAROT_EMPRESS,
  TAROT_EMPEROR,
  TAROT_HIEROPHANT,
  TAROT_LOVERS,
  TAROT_CHARIOT,
  TAROT_JUSTICE,
  TAROT_HERMIT,
  TAROT_WHEEL_OF_FORTUNE,
  TAROT_STRENGTH,
  TAROT_HANGED_MAN,
  TAROT_DEATH,
  TAROT_TEMPERANCE,
  TAROT_DEVIL,
  TAROT_TOWER,
  TAROT_STAR,
  TAROT_MOON,
  TAROT_SUN,
  TAROT_JUDGEMENT,
  TAROT_WORLD
} Tarot;

const char *get_tarot_card_name(Tarot tarot);
const char *get_tarot_card_description(Tarot tarot);

uint8_t get_tarot_max_selected(Tarot tarot);
uint8_t use_tarot_card(Tarot tarot);

#endif
