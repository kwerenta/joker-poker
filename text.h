#ifndef TEXT_H
#define TEXT_H

#include "game.h"

char *get_poker_hand_name(uint16_t hand_union);
char *get_planet_card_name(Planet planet);

char *get_card_suit_name(Suit suit);
char *get_card_rank_name(Rank rank);
int get_full_card_name(char *out, Suit suit, Rank rank);

char *get_booster_pack_size_name(BoosterPackSize size);
char *get_booster_pack_type_name(BoosterPackType type);
int get_full_booster_pack_name(char *out, BoosterPackSize size, BoosterPackType type);

#endif
