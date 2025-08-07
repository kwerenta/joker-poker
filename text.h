#ifndef TEXT_H
#define TEXT_H

#include <clay.h>

#include "game.h"

char *get_poker_hand_name(uint16_t hand_union);
char *get_planet_card_name(Planet planet);

char *get_card_suit_name(Suit suit);
char *get_card_rank_name(Rank rank);
Clay_String get_full_card_name(Suit suit, Rank rank);

char *get_booster_pack_size_name(BoosterPackSize size);
char *get_booster_pack_type_name(BoosterPackType type);
char *get_booster_pack_description_suffix(BoosterPackType type);
Clay_String get_full_booster_pack_name(BoosterPackSize size, BoosterPackType type);

char *get_voucher_name(Voucher voucher);
char *get_voucher_description(Voucher voucher);

char *get_deck_name(Deck deck);
char *get_deck_description(Deck deck);

char *get_stake_name(Stake stake);
char *get_stake_description(Stake stake);

char *get_blind_name(BlindType blind_type);

#endif
