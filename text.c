#include "text.h"

#include "game.h"
#include "state.h"

char *get_poker_hand_name(uint16_t hand_union) {
  switch (get_poker_hand(hand_union)) {
    case HAND_FLUSH_FIVE:
      return "Flush Five";
    case HAND_FLUSH_HOUSE:
      return "Flush House";
    case HAND_FIVE_OF_KIND:
      return "Five of Kind";
    case HAND_STRAIGHT_FLUSH:
      return "Straight Flush";
    case HAND_FOUR_OF_KIND:
      return "Four of Kind";
    case HAND_FULL_HOUSE:
      return "Full House";
    case HAND_FLUSH:
      return "Flush";
    case HAND_STRAIGHT:
      return "Straight";
    case HAND_THREE_OF_KIND:
      return "Three of Kind";
    case HAND_TWO_PAIR:
      return "Two Pair";
    case HAND_PAIR:
      return "Pair";
    case HAND_HIGH_CARD:
      return "High Card";
  }
}

char *get_planet_card_name(Planet planet) {
  switch (planet) {
    case PLANET_ERIS:
      return "Eris";
    case PLANET_CERES:
      return "Ceres";
    case PLANET_X:
      return "Planet X";
    case PLANET_NEPTUNE:
      return "Neptune";
    case PLANET_MARS:
      return "Mars";
    case PLANET_EARTH:
      return "Earth";
    case PLANET_JUPITER:
      return "Jupiter";
    case PLANET_SATURN:
      return "Saturn";
    case PLANET_VENUS:
      return "Venus";
    case PLANET_URANUS:
      return "Uranus";
    case PLANET_MERCURY:
      return "Mercury";
    case PLANET_PLUTO:
      return "Pluto";
  }
}

char *get_card_suit_name(Suit suit) {
  switch (suit) {
    case SUIT_HEARTS:
      return "Hearts";
    case SUIT_DIAMONDS:
      return "Diamonds";
    case SUIT_SPADES:
      return "Spades";
    case SUIT_CLUBS:
      return "Clubs";
  }
}

char *get_card_rank_name(Rank rank) {
  switch (rank) {
    case RANK_ACE:
      return "Ace";
    case RANK_TWO:
      return "2";
    case RANK_THREE:
      return "3";
    case RANK_FOUR:
      return "4";
    case RANK_FIVE:
      return "5";
    case RANK_SIX:
      return "6";
    case RANK_SEVEN:
      return "7";
    case RANK_EIGHT:
      return "8";
    case RANK_NINE:
      return "9";
    case RANK_TEN:
      return "10";
    case RANK_JACK:
      return "Jack";
    case RANK_QUEEN:
      return "Queen";
    case RANK_KING:
      return "King";
  }
}

Clay_String get_full_card_name(Suit suit, Rank rank) {
  Clay_String card_name;
  append_clay_string(&card_name, "%s of %s", get_card_rank_name(rank), get_card_suit_name(suit));

  return card_name;
}

char *get_booster_pack_size_name(BoosterPackSize size) {
  switch (size) {
    case BOOSTER_PACK_NORMAL:
      return "";
    case BOOSTER_PACK_JUMBO:
      return "Jumbo";
    case BOOSTER_PACK_MEGA:
      return "Mega";
  }
}

char *get_booster_pack_type_name(BoosterPackType type) {
  switch (type) {
    case BOOSTER_PACK_STANDARD:
      return "Standard";
    case BOOSTER_PACK_BUFFON:
      return "Buffon";
    case BOOSTER_PACK_CELESTIAL:
      return "Celestial";
    case BOOSTER_PACK_ARCANA:
      return "Arcana";
    case BOOSTER_PACK_SPECTRAL:
      return "Spectral";
  }
}

char *get_booster_pack_description_suffix(BoosterPackType type) {
  switch (type) {
    case BOOSTER_PACK_STANDARD:
      return "Playing cards to add to your deck";
    case BOOSTER_PACK_BUFFON:
      return "Joker cards";
    case BOOSTER_PACK_CELESTIAL:
      return "Planet cards to be used immediately";
    case BOOSTER_PACK_ARCANA:
      return "Tarot cards to be used immediately";
    case BOOSTER_PACK_SPECTRAL:
      return "Spectral cards to be used immediately";
  }
}

Clay_String get_full_booster_pack_name(BoosterPackSize size, BoosterPackType type) {
  Clay_String booster_pack_name;
  append_clay_string(&booster_pack_name, "%s%s%s Pack", get_booster_pack_size_name(size),
                     size == BOOSTER_PACK_NORMAL ? "" : " ", get_booster_pack_type_name(type));

  return booster_pack_name;
}

char *get_voucher_name(Voucher voucher) {
  switch (voucher) {
    case VOUCHER_OVERSTOCK:
      return "Overstock";
    case VOUCHER_CLEARANCE_SALE:
      return "Clearance Sale";
    case VOUCHER_HONE:
      return "Hone NOT IMPLEMENTED";
    case VOUCHER_REROLL_SURPLUS:
      return "Reroll Surplus NOT IMPLEMENTED";
    case VOUCHER_CRYSTALL_BALL:
      return "Crystall Ball";
    case VOUCHER_TELESCOPE:
      return "Telescope";
    case VOUCHER_GRABBER:
      return "Grabber";
    case VOUCHER_WASTEFUL:
      return "Wasteful";
    case VOUCHER_TAROT_MERCHANT:
      return "Tarot Merchant NOT IMPLEMENTED";
    case VOUCHER_PLANET_MERCHANT:
      return "Planet Merchant NOT IMPLEMENTED";
    case VOUCHER_SEED_MONEY:
      return "Seed Money";
    case VOUCHER_BLANK:
      return "Blank";
    case VOUCHER_MAGIC_TRICK:
      return "Magic Trick NOT IMPLEMENTED";
    case VOUCHER_HIEROGLYPH:
      return "Hieroglyph";
    case VOUCHER_DIRECTORS_CUT:
      return "Director's Cut NOT IMPLEMENTED";
    case VOUCHER_PAINT_BRUSH:
      return "Paint Brush";

    case VOUCHER_OVERSTOCK_PLUS:
      return "Overstock Plus";
    case VOUCHER_LIQUIDATION:
      return "Liquidation";
    case VOUCHER_GLOW_UP:
      return "Glow Up NOT IMPLEMENTED";
    case VOUCHER_REROLL_GLUT:
      return "Reroll Glut NOT IMPLEMENTED";
    case VOUCHER_OMEN_GLOBE:
      return "Omen Globe NOT IMPLEMENTED";
    case VOUCHER_OBSERVATORY:
      return "Observatory";
    case VOUCHER_NACHO_TONG:
      return "Nacho Tong";
    case VOUCHER_RECYCLOMANCY:
      return "Recyclomancy";
    case VOUCHER_TAROT_TYCOON:
      return "Tarot Tycoon NOT IMPLEMENTED";
    case VOUCHER_PLANET_TYCOON:
      return "Planet Tycoon NOT IMPLEMENTED";
    case VOUCHER_MONEY_TREE:
      return "Money Tree";
    case VOUCHER_ANTIMATTER:
      return "Antimatter";
    case VOUCHER_ILLUSION:
      return "Illusion NOT IMPLEMENTED";
    case VOUCHER_PTEROGLYPH:
      return "Pteroglyph";
    case VOUCHER_RETCON:
      return "Retcon NOT IMPLEMENTED";
    case VOUCHER_PALETTE:
      return "Palette";
  }
}

char *get_voucher_description(Voucher voucher) {
  switch (voucher) {
    case VOUCHER_OVERSTOCK:
      return "+1 card slot available in shop (to 3 slots)";
    case VOUCHER_CLEARANCE_SALE:
      return "All cards and packs in shop are 25% off";
    case VOUCHER_HONE:
      return "Foil, Holographic, and Polychrome cards appear 2x more often";
    case VOUCHER_REROLL_SURPLUS:
      return "Rerolls cost $2 less";
    case VOUCHER_CRYSTALL_BALL:
      return "+1 consumable slot";
    case VOUCHER_TELESCOPE:
      return "Celestial Packs always contain the Planet card for your most played poker hand";
    case VOUCHER_GRABBER:
      return "Permanently gain +1 hand per round";
    case VOUCHER_WASTEFUL:
      return "Permanently gain +1 discard each round";
    case VOUCHER_TAROT_MERCHANT:
      return "Tarot cards appear 2X more frequently in the shop";
    case VOUCHER_PLANET_MERCHANT:
      return "Planet cards appear 2X more frequently in the shop";
    case VOUCHER_SEED_MONEY:
      return "Raise the cap on interest earned in each round to $10";
    case VOUCHER_BLANK:
      return "Does nothing?";
    case VOUCHER_MAGIC_TRICK:
      return "Playing cards can be purchased from the shop";
    case VOUCHER_HIEROGLYPH:
      return "-1 Ante, -1 hand each round";
    case VOUCHER_DIRECTORS_CUT:
      return "Reroll Boss Blind 1 time per Ante, $10 per roll";
    case VOUCHER_PAINT_BRUSH:
      return "+1 Hand Size";

    case VOUCHER_OVERSTOCK_PLUS:
      return "+1 card slot available in shop (to 4 slots)";
    case VOUCHER_LIQUIDATION:
      return "All cards and packs in shop are 50% off";
    case VOUCHER_GLOW_UP:
      return "Foil, Holographic, and Polychrome cards appear 4x more often";
    case VOUCHER_REROLL_GLUT:
      return "Rerolls cost an additional $2 less";
    case VOUCHER_OMEN_GLOBE:
      return "Spectral cards may appear in any of the Arcana Packs";
    case VOUCHER_OBSERVATORY:
      return "Planet cards in your consumable area give X1.5 Mult for their specified poker hand";
    case VOUCHER_NACHO_TONG:
      return "Permanently gain an additional +1 hand per round";
    case VOUCHER_RECYCLOMANCY:
      return "Permanently gain an additional +1 discard each round";
    case VOUCHER_TAROT_TYCOON:
      return "Tarot cards appear 4X more frequently in the shop";
    case VOUCHER_PLANET_TYCOON:
      return "Planet cards appear 4X more frequently in the shop";
    case VOUCHER_MONEY_TREE:
      return "Raise the cap on interest earned in each round to $20";
    case VOUCHER_ANTIMATTER:
      return "+1 Joker slot";
    case VOUCHER_ILLUSION:
      return "Playing cards in shop may have an Enhancement, Edition, and/or a Seal";
    case VOUCHER_PTEROGLYPH:
      return "-1 Ante, -1 discard each round";
    case VOUCHER_RETCON:
      return "Reroll Boss Blind unlimited times, $10 per roll";
    case VOUCHER_PALETTE:
      return "+1 Hand Size again";
  }
}
