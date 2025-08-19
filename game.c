#include "game.h"

#include <cvector.h>
#include <math.h>
#include <stdint.h>

#include "content/spectral.h"
#include "content/tarot.h"
#include "debug.h"
#include "random.h"
#include "state.h"
#include "utils.h"

void game_init(Deck deck, Stake stake) {
  rng_init();

  state.game.deck_type = deck;
  state.game.stake = stake;

  generate_deck();

  state.game.score = 0;
  state.game.ante = 1;
  state.game.round = 0;

  state.game.blinds[0] = (Blind){.type = BLIND_SMALL, .tag = random_max_value(23), .is_active = 1};
  state.game.blinds[1] = (Blind){.type = BLIND_BIG, .tag = random_max_value(23), .is_active = 1};
  state.game.blinds[2] = (Blind){.is_active = 1};
  roll_boss_blind();

  state.game.current_blind = &state.game.blinds[0];

  state.game.money = 4;

  state.game.hand.size = 8;
  state.game.hands.total = 4;
  state.game.discards.total = 3;

  state.game.jokers.size = 5;
  state.game.consumables.size = 2;

  state.game.shop.size = 2;
  cvector_push_back(state.game.shop.vouchers, 0);

  apply_deck_settings();

  if (stake >= STAKE_BLUE) state.game.discards.total--;

  state.game.hands.remaining = state.game.hands.total;
  state.game.discards.remaining = state.game.discards.total;

  change_stage(STAGE_SELECT_BLIND);

  state.game.fool_last_used.was_used = 0;
  state.game.played_poker_hands = 0;
  state.game.defeated_boss_blinds = 0b11;
  state.game.has_rerolled_boss = 0;
  memset(state.game.poker_hands, 0, 12 * sizeof(PokerHandStats));

  cvector_reserve(state.game.shop.booster_packs, 2);

  cvector_copy(state.game.full_deck, state.game.deck);
  cvector_reserve(state.game.hand.cards, state.game.hand.size);

  memset(&state.game.stats, 0, sizeof(Stats));

  log_message(LOG_INFO, "Game has been initialized.");
}

void game_destroy() {
  cvector_destroy(state.game.deck);
  cvector_destroy(state.game.full_deck);
  cvector_destroy(state.game.hand.cards);
  cvector_destroy(state.game.jokers.cards);
  cvector_destroy(state.game.consumables.items);
  cvector_destroy(state.game.shop.items);
  cvector_destroy(state.game.shop.booster_packs);
  cvector_destroy(state.game.booster_pack.content);
  cvector_destroy(state.game.shop.vouchers);
  cvector_destroy(state.game.tags);

  log_message(LOG_INFO, "Game has been destroyed.");
}

void generate_deck() {
  cvector_reserve(state.game.full_deck, state.game.deck_type == DECK_ABANDONED ? 40 : 52);
  for (uint8_t i = 0; i < 52; i++) {
    Rank rank = i % 13;
    Suit suit = i % 4;

    if (state.game.deck_type == DECK_ABANDONED && (rank == RANK_JACK || rank == RANK_QUEEN || rank == RANK_KING))
      continue;

    // Convert possible suit values from 0 1 2 3 to 0 2 (hearts and spades)
    if (state.game.deck_type == DECK_CHECKERED) suit = 2 * (suit % 2);

    if (state.game.deck_type == DECK_ERRATIC) {
      rank = random_max_value(12);
      suit = random_max_value(3);
    }

    cvector_push_back(state.game.full_deck, create_card(suit, rank, EDITION_BASE, ENHANCEMENT_NONE, SEAL_NONE));
  }
}

void apply_deck_settings() {
  switch (state.game.deck_type) {
    case DECK_RED:
      state.game.discards.total++;
      break;
    case DECK_BLUE:
      state.game.hands.total++;
      break;
    case DECK_YELLOW:
      state.game.money += 10;
      break;
    case DECK_GREEN:
      break;
    case DECK_BLACK:
      state.game.jokers.size++;
      state.game.hands.total--;
      break;
    case DECK_MAGIC:
      add_voucher_to_player(VOUCHER_CRYSTALL_BALL);
      ShopItem fool = {.type = SHOP_ITEM_TAROT, .tarot = TAROT_FOOL};
      add_item_to_player(&fool);
      add_item_to_player(&fool);
      break;
    case DECK_NEBULA:
      add_voucher_to_player(VOUCHER_TELESCOPE);
      state.game.consumables.size--;
      break;
    case DECK_GHOST:
      add_item_to_player(&(ShopItem){.type = SHOP_ITEM_SPECTRAL, .spectral = SPECTRAL_HEX});
      break;
    case DECK_ABANDONED:
    case DECK_CHECKERED:
      break;
    case DECK_ZODIAC:
      add_voucher_to_player(VOUCHER_TAROT_MERCHANT);
      add_voucher_to_player(VOUCHER_PLANET_MERCHANT);
      add_voucher_to_player(VOUCHER_OVERSTOCK);
      break;
    case DECK_PAINTED:
      state.game.hand.size += 2;
      state.game.jokers.size--;
      break;
    case DECK_ANAGLYPH:
    case DECK_PLASMA:
    case DECK_ERRATIC:
      break;
  }
}

uint8_t compare_cards(Card *a, Card *b) {
  return a->chips == b->chips && a->enhancement == b->enhancement && a->edition == b->edition && a->rank == b->rank &&
         a->suit == b->suit && a->seal == b->seal && a->was_played == b->was_played;
}

Card create_card(Suit suit, Rank rank, Edition edition, Enhancement enhancement, Seal seal) {
  uint16_t chips = rank == RANK_ACE ? 11 : rank + 1;
  if (rank != RANK_ACE && chips > 10) chips = 10;

  return (Card){.suit = suit,
                .rank = rank,
                .chips = chips,
                .edition = edition,
                .enhancement = enhancement,
                .seal = seal,
                .selected = 0};
}

void draw_card() {
  cvector_push_back(state.game.hand.cards, cvector_back(state.game.deck));
  cvector_pop_back(state.game.deck);

  if (state.stage == STAGE_SELECT_BLIND || state.stage == STAGE_GAME) {
    state.game.stats.drawn_cards++;

    if (!state.game.current_blind->is_active) return;

    switch (state.game.current_blind->type) {
      case BLIND_WHEEL:
        if (state.game.stats.drawn_cards % 7 == 0) cvector_back(state.game.hand.cards).status |= CARD_STATUS_FACE_DOWN;
        break;
      case BLIND_MARK:
        if (is_face_card(&cvector_back(state.game.hand.cards)))
          cvector_back(state.game.hand.cards).status |= CARD_STATUS_FACE_DOWN;
        break;
      default:
        break;
    }
  }
}

void fill_hand() {
  while (cvector_size(state.game.hand.cards) < state.game.hand.size) draw_card();
}

void trigger_scoring_card(Card *card) {
  if (card->enhancement != ENHANCEMENT_STONE) state.game.selected_hand.score_pair.chips += card->chips;

  update_scoring_edition(card->edition);

  switch (card->enhancement) {
    case ENHANCEMENT_NONE:
    case ENHANCEMENT_GOLD:
    case ENHANCEMENT_WILD:
    case ENHANCEMENT_STEEL:
      break;

    case ENHANCEMENT_BONUS:
      state.game.selected_hand.score_pair.chips += 30;
      break;

    case ENHANCEMENT_MULT:
      state.game.selected_hand.score_pair.mult += 4;
      break;

    case ENHANCEMENT_GLASS:
      state.game.selected_hand.score_pair.mult *= 2;

      if (random_chance(1, 4)) {
        for (uint8_t i = 0; i < cvector_size(state.game.full_deck); i++) {
          Card *other = &state.game.full_deck[i];
          if (compare_cards(card, other)) {
            cvector_erase(state.game.full_deck, i);
            break;
          }
        }
      }
      break;

    case ENHANCEMENT_STONE:
      state.game.selected_hand.score_pair.chips += 50;
      break;

    case ENHANCEMENT_LUCKY:
      if (random_chance(1, 5)) state.game.selected_hand.score_pair.mult += 20;
      if (random_chance(1, 15)) state.game.money += 20;
      break;
  }

  if (card->seal == SEAL_GOLD) state.game.money += 3;
}

void trigger_in_hand_card(Card *card) {
  if (card->enhancement == ENHANCEMENT_STEEL) state.game.selected_hand.score_pair.mult *= 1.5;
}

void trigger_end_of_round_card(Card *card) {
  if (card->enhancement == ENHANCEMENT_GOLD) state.game.money += 3;
  if (card->seal == SEAL_BLUE)
    add_item_to_player(&(ShopItem){.type = SHOP_ITEM_PLANET, .planet = ffs(state.game.selected_hand.hand_union) - 1});
}

void play_hand() {
  if (state.game.hands.remaining == 0 || state.game.selected_hand.count == 0) return;

  update_scoring_hand();

  if (state.game.current_blind->is_active) {
    switch (state.game.current_blind->type) {
      case BLIND_OX:
        if (get_poker_hand(state.game.selected_hand.hand_union) == get_most_played_poker_hand()) state.game.money = 0;
        break;
      case BLIND_ARM: {
        PokerHandStats *played_hand_stats = get_poker_hand_stats(state.game.selected_hand.hand_union);
        if (played_hand_stats->level >= 1) played_hand_stats->level--;
        break;
      }
      case BLIND_PSYCHIC:
        if (state.game.selected_hand.count != 5) {
          replace_selected_cards();
          return;
        }
        break;
      case BLIND_EYE:
        if (state.game.played_poker_hands & get_poker_hand(state.game.selected_hand.hand_union)) {
          replace_selected_cards();
          return;
        }
        break;
      case BLIND_MOUTH:
        if (state.game.played_poker_hands &&
            !(state.game.played_poker_hands & get_poker_hand(state.game.selected_hand.hand_union))) {
          replace_selected_cards();
          return;
        }
        break;
      case BLIND_TOOTH:
        state.game.money -= state.game.selected_hand.count;
        break;
      case BLIND_FLINT:
        state.game.selected_hand.score_pair.mult /= 2;
        state.game.selected_hand.score_pair.chips /= 2;
        break;
      default:
        break;
    }
  }

  get_poker_hand_stats(state.game.selected_hand.hand_union)->played++;
  state.game.played_poker_hands |= get_poker_hand(state.game.selected_hand.hand_union);

  for (uint8_t i = 0; i < 5; i++) {
    Card *card = state.game.selected_hand.scoring_cards[i];
    if (card == NULL || card->status & CARD_STATUS_DEBUFFED) continue;

    trigger_scoring_card(card);
    if (card->seal == SEAL_RED) trigger_scoring_card(card);
  }

  cvector_for_each(state.game.jokers.cards, Joker, joker) {
    if (joker->status & CARD_STATUS_DEBUFFED) continue;

    if (joker->activation_type == ACTIVATION_INDEPENDENT) joker->activate();
    update_scoring_edition(joker->edition);
  }

  if (state.game.current_blind->type <= BLIND_BIG) {
    cvector_for_each(state.game.hand.cards, Card, card) {
      cvector_for_each(state.game.full_deck, Card, deck_card) {
        if (card->selected > 0 && compare_cards(card, deck_card)) {
          deck_card->was_played = 1;
          break;
        }
      }
    }
  }

  uint8_t cards_played = state.game.selected_hand.count;
  remove_selected_cards();
  state.game.hands.remaining--;

  if (state.game.current_blind->is_active && state.game.current_blind->type == BLIND_HOOK) {
    for (uint8_t i = 0; i < 2; i++) discard_card(random_vector_index(state.game.hand.cards));
  }

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->status & CARD_STATUS_DEBUFFED) continue;

    trigger_in_hand_card(card);
    if (card->seal == SEAL_RED) trigger_in_hand_card(card);
  }

  if (state.game.vouchers & VOUCHER_OBSERVATORY) {
    cvector_for_each(state.game.consumables.items, Consumable, consumable) {
      if (consumable->type == CONSUMABLE_PLANET && (1 << consumable->planet) == state.game.selected_hand.hand_union) {
        state.game.selected_hand.score_pair.mult *= 1.5;
      }
    }
  }

  if (state.game.deck_type == DECK_PLASMA)
    state.game.score +=
        pow(floor((state.game.selected_hand.score_pair.chips + state.game.selected_hand.score_pair.mult) / 2), 2);
  else
    state.game.score += state.game.selected_hand.score_pair.chips * state.game.selected_hand.score_pair.mult;

  double required_score = get_required_score(state.game.ante, state.game.current_blind->type);

  if (state.game.score >= required_score) {
    cvector_for_each(state.game.hand.cards, Card, card) {
      if (card->status & CARD_STATUS_DEBUFFED) continue;

      trigger_end_of_round_card(card);
      if (card->seal == SEAL_RED) trigger_end_of_round_card(card);
    }

    state.game.stats.hands.total +=
        state.game.current_blind->is_active && state.game.current_blind->type == BLIND_NEEDLE ? 1
                                                                                              : state.game.hands.total;
    state.game.stats.hands.remaining += state.game.hands.remaining;
    state.game.stats.discards.total +=
        state.game.current_blind->is_active && state.game.current_blind->type == BLIND_WATER
            ? 0
            : state.game.discards.total;
    state.game.stats.discards.remaining += state.game.discards.remaining;

    change_stage(STAGE_CASH_OUT);
  } else if (state.game.hands.remaining == 0) {
    change_stage(STAGE_GAME_OVER);
  } else {
    if (state.game.current_blind->is_active && state.game.current_blind->type == BLIND_SERPENT)
      for (uint8_t i = 0; i < 3; i++) draw_card();
    else
      fill_hand();

    if (state.game.current_blind->is_active && state.game.current_blind->type == BLIND_FISH)
      for (int8_t i = cards_played; i > 0; i--)
        state.game.hand.cards[cvector_size(state.game.hand.cards) - i].status |= CARD_STATUS_FACE_DOWN;

    sort_hand();

    if (!state.game.current_blind->is_active) return;

    if (state.game.current_blind->type == BLIND_CRIMSON_HEART) {
      disable_boss_blind();
      enable_boss_blind();
    } else if (state.game.current_blind->type == BLIND_CERULEAN_BELL) {
      force_card_select(random_vector_index(state.game.hand.cards));
    }
  }
}

void cash_out() {
  state.game.money += get_interest_money() + get_hands_money() + get_discards_money() +
                      get_blind_money(state.game.current_blind->type) + get_investment_tag_money();

  state.game.score = 0;
  state.game.played_poker_hands = 0;

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == TAG_JUGGLE) {
      state.game.hand.size -= 3;
      cvector_erase(state.game.tags, i);
      i--;
    }
  }

  if (state.game.current_blind->type > BLIND_BIG) {
    disable_boss_blind();
    cvector_for_each(state.game.full_deck, Card, card) card->was_played = 0;
    state.game.defeated_boss_blinds |= 1 << state.game.current_blind->type;

    state.game.ante++;
    state.game.has_rerolled_boss = 0;

    for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
      if (state.game.tags[i] == TAG_INVESTMENT) {
        cvector_erase(state.game.tags, i);
        i--;
      }
    }

    if (state.game.deck_type == DECK_ANAGLYPH) cvector_push_back(state.game.tags, TAG_DOUBLE);

    state.game.current_blind = &state.game.blinds[0];
    for (uint8_t i = 0; i < 3; i++) {
      state.game.blinds[i].is_active = 1;
      if (i != 2) state.game.blinds[i].tag = random_max_value(23);
    }

    roll_boss_blind();
  } else {
    state.game.current_blind++;
  }

  // Reset hand and deck for new blind
  state.game.hands.remaining = state.game.hands.total;
  state.game.discards.remaining = state.game.discards.total;
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck, state.game.deck);

  change_stage(STAGE_SHOP);
  restock_shop();
}

void update_scoring_edition(Edition edition) {
  switch (edition) {
    case EDITION_BASE:
      break;
    case EDITION_FOIL:
      state.game.selected_hand.score_pair.chips += 50;
      break;
    case EDITION_HOLOGRAPHIC:
      state.game.selected_hand.score_pair.mult += 10;
      break;
    case EDITION_POLYCHROME:
      state.game.selected_hand.score_pair.mult *= 1.5;
      break;
    case EDITION_NEGATIVE:
      // TODO implement negative jokers and cards when consumable slots will be added
      break;
  }
}

void discard_hand() {
  if (state.game.selected_hand.count == 0 || state.game.discards.remaining == 0) return;

  for (uint8_t i = 0; i < cvector_size(state.game.hand.cards); i++) {
    if (state.game.hand.cards[i].selected > 0) {
      discard_card(i);
      i--;
    }
  }

  state.game.selected_hand.count = 0;
  state.game.discards.remaining--;

  if (state.game.current_blind->is_active && state.game.current_blind->type == BLIND_SERPENT)
    for (uint8_t i = 0; i < 3; i++) draw_card();
  else
    fill_hand();
  sort_hand();

  if (state.game.current_blind->is_active && state.game.current_blind->type == BLIND_CERULEAN_BELL)
    force_card_select(random_vector_index(state.game.hand.cards));
}

void remove_selected_cards() {
  uint8_t i = 0;
  while (i < cvector_size(state.game.hand.cards)) {
    if (state.game.hand.cards[i].selected > 0) {
      cvector_erase(state.game.hand.cards, i);
      continue;
    }

    i++;
  }

  state.game.selected_hand.count = 0;
}

void replace_selected_cards() {
  remove_selected_cards();
  state.game.hands.remaining--;

  if (state.game.hands.remaining == 0) {
    change_stage(STAGE_GAME_OVER);
    return;
  }

  fill_hand();
  sort_hand();
}

void discard_card(uint8_t index) {
  if (index >= cvector_size(state.game.hand.cards)) return;

  if (!(state.game.hand.cards[index].status & CARD_STATUS_DEBUFFED) && state.game.hand.cards[index].seal == SEAL_PURPLE)
    add_item_to_player(&(ShopItem){.type = SHOP_ITEM_TAROT, .tarot = random_max_value(21)});

  cvector_erase(state.game.hand.cards, index);
}

uint8_t is_face_card(Card *card) {
  if (card->rank >= RANK_JACK && card->rank <= RANK_KING) return 1;
  return 0;
}

uint8_t is_suit(Card *card, Suit suit) {
  if (card->enhancement == ENHANCEMENT_WILD) return 1;
  return card->suit == suit;
}

uint8_t is_poker_hand_unknown() {
  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected > 0 && card->status & CARD_STATUS_FACE_DOWN) return 1;
  }
  return 0;
}

bool is_planet_card_locked(Planet planet) { return planet <= PLANET_X && state.game.poker_hands[planet].played == 0; }
bool filter_locked_planet_cards(uint8_t planet) { return !is_planet_card_locked(planet); }

void shuffle_deck() {
  for (uint8_t i = cvector_size(state.game.deck) - 1; i > 0; i--) {
    uint8_t j = rand() % (i + 1);
    Card temp = state.game.deck[i];
    state.game.deck[i] = state.game.deck[j];
    state.game.deck[j] = temp;
  }
}

void toggle_card_select(uint8_t index) {
  Hand *hand = &state.game.hand;

  if (hand->cards[index].selected == 2) return;

  if (hand->cards[index].selected == 1) {
    hand->cards[index].selected = 0;
    state.game.selected_hand.count--;
    update_scoring_hand();
    return;
  }

  uint8_t *selected_count = &state.game.selected_hand.count;
  *selected_count = 0;
  cvector_for_each(hand->cards, Card, card) {
    if (card->selected == 0) continue;

    (*selected_count)++;

    if (*selected_count == 5) return;
  }

  (*selected_count)++;

  hand->cards[index].selected = 1;

  update_scoring_hand();
}

void force_card_select(uint8_t index) {
  toggle_card_select(index);
  state.game.hand.cards[index].selected = 2;
}

void deselect_all_cards() {
  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected == 1) card->selected = 0;
  }
  state.game.selected_hand.count = 0;
}

int compare_by_rank(const void *a, const void *b) {
  Rank a_rank = ((const Card *)a)->rank, b_rank = ((const Card *)b)->rank;
  if (a_rank == RANK_ACE) a_rank = RANK_KING + 1;
  if (b_rank == RANK_ACE) b_rank = RANK_KING + 1;

  int by_rank = b_rank - a_rank;
  if (by_rank == 0) return ((const Card *)a)->suit - ((const Card *)b)->suit;

  return by_rank;
}
int compare_by_suit(const void *a, const void *b) {
  int by_suit = ((const Card *)a)->suit - ((const Card *)b)->suit;
  if (by_suit == 0) return compare_by_rank(a, b);

  return by_suit;
}

void sort_hand() {
  int (*comparator)(const void *a, const void *b) = compare_by_rank;
  if (state.game.sorting_mode == SORTING_BY_SUIT) comparator = compare_by_suit;

  qsort(state.game.hand.cards, cvector_size(state.game.hand.cards), sizeof(Card), comparator);
}

uint16_t evaluate_hand() {
  const Hand *hand = &state.game.hand;

  uint16_t result = HAND_HIGH_CARD;

  uint8_t rank_counts[13] = {};
  uint8_t suit_counts[4] = {};
  const uint8_t selected_count = state.game.selected_hand.count;

  // 2 of kind, 3 of kind, 4 of kind, 5 of kind
  uint8_t x_of_kind[4] = {};
  uint8_t is_straight_possible = 1;
  uint8_t has_ace = 0;

  Rank highest_card = RANK_TWO, lowest_card = RANK_KING;

  cvector_for_each(hand->cards, Card, card) {
    if (card->selected == 0 || card->enhancement == ENHANCEMENT_STONE) continue;

    if (card->rank == RANK_ACE) has_ace = 1;
    if (card->rank != RANK_ACE && card->rank > highest_card) highest_card = card->rank;
    if (card->rank != RANK_ACE && card->rank < lowest_card) lowest_card = card->rank;

    rank_counts[card->rank]++;

    if (card->enhancement == ENHANCEMENT_WILD)
      for (uint8_t i = 0; i < 4; i++) suit_counts[i]++;
    else
      suit_counts[card->suit]++;

    if (rank_counts[card->rank] > 1) {
      // this means there are duplicates in selected ranks,
      // so straight is not possible
      is_straight_possible = 0;
      x_of_kind[rank_counts[card->rank] - 2]++;
      if (rank_counts[card->rank] > 2) x_of_kind[rank_counts[card->rank] - 3]--;
    }
  }

  for (uint8_t i = 0; i < 4; i++)
    if (suit_counts[i] == 5) result |= HAND_FLUSH;

  // A 2 3 4 5 is also valid straight, so it needs to be checked
  if (is_straight_possible == 1 && selected_count == 5 &&
      ((has_ace == 0 && highest_card - lowest_card == 4) ||
       (has_ace == 1 && (13 - lowest_card == 4 || highest_card == 4))))
    result |= HAND_STRAIGHT;

  for (uint8_t i = 2; i <= 5; i++) {
    if (x_of_kind[i - 2] == 0) continue;

    if (i >= 2) result |= HAND_PAIR;
    if (i >= 3) result |= HAND_THREE_OF_KIND;
    if (i >= 4) result |= HAND_FOUR_OF_KIND;
    if (i >= 5) result |= HAND_FIVE_OF_KIND;
  }

  if (x_of_kind[2 - 2] >= 2) result |= HAND_TWO_PAIR;
  if (x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1) result |= HAND_FULL_HOUSE;

  if ((result & HAND_FLUSH) != 0 && (result & HAND_STRAIGHT) != 0) result |= HAND_STRAIGHT_FLUSH;
  if ((result & HAND_FLUSH) != 0 && (result & HAND_FULL_HOUSE) != 0) result |= HAND_FLUSH_HOUSE;
  if ((result & HAND_FLUSH) != 0 && (result & HAND_FIVE_OF_KIND) != 0) result |= HAND_FLUSH_FIVE;

  return result;
}

uint8_t does_poker_hand_contain(uint16_t hand_union, PokerHand expected) {
  if ((hand_union & expected) != 0) return 1;
  return 0;
}

PokerHand get_poker_hand(uint16_t hand_union) { return 1 << (ffs(hand_union) - 1); }

void update_scoring_hand() {
  uint16_t hand_union = evaluate_hand();

  state.game.selected_hand.hand_union = hand_union;
  const Hand *hand = &state.game.hand;
  Card *selected_cards[5] = {};
  Card **scoring_cards = state.game.selected_hand.scoring_cards;

  // Clear previous scoring cards
  memset(scoring_cards, 0, 5 * sizeof(Card *));

  uint8_t j = 0;
  cvector_for_each(hand->cards, Card, card) {
    if (card->selected == 0) continue;

    selected_cards[j] = card;
    j++;
  }

  if (selected_cards[0] == NULL) return;

  // All of those hands require 5 selected cards
  if (does_poker_hand_contain(hand_union, HAND_FULL_HOUSE) || does_poker_hand_contain(hand_union, HAND_FLUSH) ||
      does_poker_hand_contain(hand_union, HAND_STRAIGHT) || does_poker_hand_contain(hand_union, HAND_FIVE_OF_KIND)) {
    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL) return;

      scoring_cards[i] = selected_cards[i];
    }
  }

  uint8_t rank_counts[13] = {};
  uint8_t highest_card_index = 0xFF;
  Rank scoring_rank = selected_cards[0]->rank;
  uint8_t scoring_count = 0;
  for (uint8_t i = 0; i < 5; i++) {
    if (selected_cards[i] == NULL) break;

    if (selected_cards[i]->enhancement == ENHANCEMENT_STONE) continue;

    if (highest_card_index == 0xFF ||
        (selected_cards[highest_card_index]->rank != RANK_ACE &&
         (selected_cards[i]->rank == RANK_ACE || selected_cards[i]->rank > selected_cards[highest_card_index]->rank)))
      highest_card_index = i;

    rank_counts[selected_cards[i]->rank]++;

    if (rank_counts[selected_cards[i]->rank] > scoring_count) {
      scoring_rank = selected_cards[i]->rank;
      scoring_count = rank_counts[scoring_rank];
    }
  }

  if (does_poker_hand_contain(hand_union, HAND_PAIR) && does_poker_hand_contain(hand_union, HAND_FULL_HOUSE) == 0) {
    uint8_t j = 0;

    Rank second_scoring_rank = scoring_rank;
    if (get_poker_hand(hand_union) == HAND_TWO_PAIR) {
      for (uint8_t i = 0; i < 13; i++) {
        if (rank_counts[i] == 2 && i != scoring_rank) {
          second_scoring_rank = i;
          break;
        }
      }
    }

    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL ||
          (selected_cards[i]->rank != scoring_rank && selected_cards[i]->rank != second_scoring_rank &&
           selected_cards[i]->enhancement != ENHANCEMENT_STONE))
        continue;

      scoring_cards[j] = selected_cards[i];
      j++;
    }
  }

  j = 0;
  if (get_poker_hand(hand_union) == HAND_HIGH_CARD) {
    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL || (i != highest_card_index && selected_cards[i]->enhancement != ENHANCEMENT_STONE))
        continue;

      scoring_cards[j] = selected_cards[i];
      j++;
    }
  }

  state.game.selected_hand.score_pair = get_poker_hand_total_score(hand_union);
}

ScorePair get_poker_hand_base_score(uint16_t hand_union) {
  switch (get_poker_hand(hand_union)) {
    case HAND_FLUSH_FIVE:
      return (ScorePair){.mult = 16, .chips = 160};
    case HAND_FLUSH_HOUSE:
      return (ScorePair){.mult = 14, .chips = 140};
    case HAND_FIVE_OF_KIND:
      return (ScorePair){.mult = 12, .chips = 120};
    case HAND_STRAIGHT_FLUSH:
      return (ScorePair){.mult = 8, .chips = 100};
    case HAND_FOUR_OF_KIND:
      return (ScorePair){.mult = 7, .chips = 60};
    case HAND_FULL_HOUSE:
      return (ScorePair){.mult = 4, .chips = 40};
    case HAND_FLUSH:
      return (ScorePair){.mult = 4, .chips = 35};
    case HAND_STRAIGHT:
      return (ScorePair){.mult = 4, .chips = 30};
    case HAND_THREE_OF_KIND:
      return (ScorePair){.mult = 3, .chips = 30};
    case HAND_TWO_PAIR:
      return (ScorePair){.mult = 2, .chips = 20};
    case HAND_PAIR:
      return (ScorePair){.mult = 2, .chips = 10};
    case HAND_HIGH_CARD:
      return (ScorePair){.mult = 1, .chips = 5};
  }
}

ScorePair get_planet_card_base_score(uint16_t hand_union) {
  switch (get_poker_hand(hand_union)) {
    case HAND_FLUSH_FIVE:
      return (ScorePair){.mult = 3, .chips = 50};
    case HAND_FLUSH_HOUSE:
      return (ScorePair){.mult = 4, .chips = 40};
    case HAND_FIVE_OF_KIND:
      return (ScorePair){.mult = 3, .chips = 35};
    case HAND_STRAIGHT_FLUSH:
      return (ScorePair){.mult = 4, .chips = 40};
    case HAND_FOUR_OF_KIND:
      return (ScorePair){.mult = 3, .chips = 30};
    case HAND_FULL_HOUSE:
      return (ScorePair){.mult = 2, .chips = 25};
    case HAND_FLUSH:
      return (ScorePair){.mult = 2, .chips = 15};
    case HAND_STRAIGHT:
      return (ScorePair){.mult = 3, .chips = 30};
    case HAND_THREE_OF_KIND:
      return (ScorePair){.mult = 2, .chips = 20};
    case HAND_TWO_PAIR:
      return (ScorePair){.mult = 1, .chips = 20};
    case HAND_PAIR:
      return (ScorePair){.mult = 1, .chips = 15};
    case HAND_HIGH_CARD:
      return (ScorePair){.mult = 1, .chips = 10};
  }
}

PokerHandStats *get_poker_hand_stats(uint16_t hand_union) { return &state.game.poker_hands[ffs(hand_union) - 1]; }

ScorePair get_poker_hand_total_score(uint16_t hand_union) {
  ScorePair poker_hand_score = get_poker_hand_base_score(hand_union);
  ScorePair planet_score = get_planet_card_base_score(hand_union);

  uint8_t poker_hand_level = get_poker_hand_stats(hand_union)->level;
  poker_hand_score.chips += planet_score.chips * poker_hand_level;
  poker_hand_score.mult += planet_score.mult * poker_hand_level;

  return poker_hand_score;
}

double get_ante_base_score(uint8_t ante) {
  if (state.game.stake >= STAKE_PURPLE) {
    switch (ante) {
      case 0:
        return 100;
      case 1:
        return 300;
      case 2:
        return 1000;
      case 3:
        return 3200;
      case 4:
        return 9000;
      case 5:
        return 25000;
      case 6:
        return 60000;
      case 7:
        return 110000;
      case 8:
        return 200000;
    }
  }

  if (state.game.stake >= STAKE_GREEN) {
    switch (ante) {
      case 0:
        return 100;
      case 1:
        return 300;
      case 2:
        return 900;
      case 3:
        return 2600;
      case 4:
        return 8000;
      case 5:
        return 20000;
      case 6:
        return 36000;
      case 7:
        return 60000;
      case 8:
        return 100000;
    }
  }

  switch (ante) {
    case 0:
      return 100;
    case 1:
      return 300;
    case 2:
      return 800;
    case 3:
      return 2000;
    case 4:
      return 5000;
    case 5:
      return 11000;
    case 6:
      return 20000;
    case 7:
      return 35000;
    case 8:
      return 50000;
  }

  return 0;
}

double get_required_score(uint8_t ante, BlindType blind_type) {
  double base_score = (state.game.deck_type == DECK_PLASMA ? 2 : 1) * get_ante_base_score(ante);

  switch (blind_type) {
    case BLIND_SMALL:
      return base_score;
    case BLIND_BIG:
      return 1.5 * base_score;

    case BLIND_WALL:
      return (state.game.current_blind->is_active ? 4.0 : 2.0) * base_score;
    case BLIND_NEEDLE:
      return base_score;
    case BLIND_VIOLET_VESSEL:
      return (state.game.current_blind->is_active ? 6.0 : 2.0) * base_score;

    default:
      return 2.0 * base_score;
  }
}

uint8_t get_blind_money(BlindType blind_type) {
  // BLIND_AMBER_ACORN is the first Finisher Boss Blind, they have higher reward
  return blind_type == BLIND_SMALL        ? state.game.stake >= STAKE_RED ? 0 : 3
         : blind_type == BLIND_BIG        ? 4
         : blind_type < BLIND_AMBER_ACORN ? 5
                                          : 8;
}
uint8_t get_hands_money() { return (state.game.deck_type == DECK_GREEN ? 2 : 1) * state.game.hands.remaining; }
uint8_t get_discards_money() { return (state.game.deck_type == DECK_GREEN ? 1 : 0) * state.game.discards.remaining; }
uint8_t get_investment_tag_money() {
  if (state.game.current_blind->type <= BLIND_BIG) return 0;

  uint8_t total = 0;
  cvector_for_each(state.game.tags, Tag, tag) {
    if (*tag == TAG_INVESTMENT) total += 25;
  }

  return total;
}

uint8_t get_interest_money() {
  if (state.game.deck_type == DECK_GREEN) return 0;

  uint8_t interest = state.game.money / 5;
  uint8_t interest_cap = state.game.vouchers & VOUCHER_MONEY_TREE   ? 20
                         : state.game.vouchers & VOUCHER_SEED_MONEY ? 10
                                                                    : 5;

  if (interest > interest_cap) return interest_cap;

  return interest;
}

// If consumable is NULL then hovered item from consumables section will be used
uint8_t use_consumable(Consumable *consumable_to_use) {
  Consumable consumable;

  if (consumable_to_use == NULL) {
    if (cvector_size(state.game.consumables.items) == 0) return 0;

    consumable = cvector_at(state.game.consumables.items, state.navigation.hovered);
    cvector_erase(state.game.consumables.items, state.navigation.hovered);
  } else {
    consumable = *consumable_to_use;
  }

  uint8_t was_used = 1;
  switch (consumable.type) {
    case CONSUMABLE_PLANET:
      state.game.poker_hands[consumable.planet].level += 1;
      break;

    case CONSUMABLE_TAROT:
      was_used = use_tarot_card(consumable.tarot);
      break;

    case CONSUMABLE_SPECTRAL:
      was_used = use_spectral_card(consumable.spectral);
      break;
  }

  if (was_used == 0 && consumable_to_use == NULL) {
    cvector_insert(state.game.consumables.items, state.navigation.hovered, consumable);
    return 0;
  }

  if (consumable.type != CONSUMABLE_TAROT || consumable.tarot != TAROT_FOOL) {
    state.game.fool_last_used.was_used = 1;
    state.game.fool_last_used.consumable = consumable;
  }

  if (consumable_to_use == NULL) set_nav_hovered(state.navigation.hovered);
  return was_used;
}

uint8_t add_item_to_player(ShopItem *item) {
  switch (item->type) {
    case SHOP_ITEM_JOKER:
      if (cvector_size(state.game.jokers.cards) >= state.game.jokers.size) return 0;

      cvector_push_back(state.game.jokers.cards, item->joker);
      break;

    case SHOP_ITEM_CARD:
      cvector_push_back(state.game.full_deck, item->card);
      break;

    case SHOP_ITEM_PLANET:
      if (cvector_size(state.game.consumables.items) >= state.game.consumables.size) return 0;

      Consumable planet = {.type = CONSUMABLE_PLANET, .planet = item->planet};
      cvector_push_back(state.game.consumables.items, planet);
      break;

    case SHOP_ITEM_TAROT:
      if (cvector_size(state.game.consumables.items) >= state.game.consumables.size) return 0;

      Consumable tarot = {.type = CONSUMABLE_TAROT, .tarot = item->tarot};
      cvector_push_back(state.game.consumables.items, tarot);
      break;

    case SHOP_ITEM_SPECTRAL:
      if (cvector_size(state.game.consumables.items) >= state.game.consumables.size) return 0;

      Consumable spectral = {.type = CONSUMABLE_SPECTRAL, .spectral = item->spectral};
      cvector_push_back(state.game.consumables.items, spectral);
      break;
  }

  return 1;
}

uint8_t apply_sale(uint8_t price) {
  float sale = 0.0f;

  if (state.game.vouchers & VOUCHER_LIQUIDATION)
    sale = 0.5f;
  else if (state.game.vouchers & VOUCHER_CLEARANCE_SALE)
    sale = 0.25f;

  return (uint8_t)ceilf((1 - sale) * price - 0.5f);
}

uint8_t get_shop_item_price(ShopItem *item) {
  cvector_for_each(state.game.tags, Tag, tag) {
    if (*tag == TAG_COUPON) return 0;
  }

  uint8_t price = 0;
  switch (item->type) {
    case SHOP_ITEM_CARD:
      price = 1;
      break;
    case SHOP_ITEM_PLANET:
    case SHOP_ITEM_TAROT:
      price = 3;
      break;
    case SHOP_ITEM_SPECTRAL:
      price = 4;
      break;
    case SHOP_ITEM_JOKER:
      price = item->joker.base_price;
      break;
  }

  return apply_sale(price);
}

uint8_t get_voucher_price(Voucher voucher) {
  if (voucher <= 1 << 16) return 10;
  return 20;
}
void add_voucher_to_player(Voucher voucher) {
  state.game.vouchers |= voucher;

  switch (voucher) {
    case VOUCHER_OVERSTOCK:
      state.game.shop.size = 3;
      fill_shop_items();
      break;
    case VOUCHER_CRYSTALL_BALL:
      state.game.consumables.size++;
      break;
    case VOUCHER_GRABBER:
      state.game.hands.total++;
      break;
    case VOUCHER_WASTEFUL:
      state.game.discards.total++;
      break;
    case VOUCHER_HIEROGLYPH:
      state.game.ante--;
      state.game.hands.total--;
      break;
    case VOUCHER_PAINT_BRUSH:
      state.game.hand.size++;
      break;

    case VOUCHER_OVERSTOCK_PLUS:
      state.game.shop.size = 4;
      fill_shop_items();
      break;
    case VOUCHER_NACHO_TONG:
      state.game.hands.total++;
      break;
    case VOUCHER_RECYCLOMANCY:
      state.game.discards.total++;
      break;
    case VOUCHER_ANTIMATTER:
      state.game.jokers.size++;
      break;
    case VOUCHER_PTEROGLYPH:
      state.game.ante--;
      state.game.discards.total--;
      break;
    case VOUCHER_PALETTE:
      state.game.hand.size++;
      break;

    default:
      break;
  }
}

uint8_t get_booster_pack_price(BoosterPackItem *booster_pack) {
  cvector_for_each(state.game.tags, Tag, tag) {
    if (*tag == TAG_COUPON) return 0;
  }

  return apply_sale(4 + booster_pack->size * 2);
}
uint8_t get_booster_pack_items_count(BoosterPackItem *booster_pack) {
  uint8_t count = booster_pack->size == BOOSTER_PACK_NORMAL ? 3 : 5;
  if (booster_pack->type == BOOSTER_PACK_BUFFON) count--;
  return count;
}

uint8_t get_shop_item_sell_price(ShopItem *item) {
  int8_t sell_price = (uint8_t)floorf(get_shop_item_price(item) / 2.0);

  if (sell_price < 1) return 1;
  return sell_price;
}

static bool buy_booster_pack(uint8_t index) {
  open_booster_pack(&state.game.shop.booster_packs[index]);
  cvector_erase(state.game.shop.booster_packs, index);
  return true;
}

static bool buy_voucher(uint8_t index) {
  if (index == cvector_size(state.game.shop.vouchers) - 1)
    state.game.shop.vouchers[index] = 0;
  else
    cvector_erase(state.game.shop.vouchers, index);
  return true;
}

static bool buy_shop_item(uint8_t index, bool should_use) {
  ShopItem *item = &state.game.shop.items[index];

  if (should_use) {
    Consumable c;
    switch (item->type) {
      case SHOP_ITEM_TAROT:
        c = (Consumable){.type = CONSUMABLE_TAROT, .tarot = item->tarot};
        break;
      case SHOP_ITEM_PLANET:
        c = (Consumable){.type = CONSUMABLE_PLANET, .planet = item->planet};
        break;
      case SHOP_ITEM_SPECTRAL:
        c = (Consumable){.type = CONSUMABLE_SPECTRAL, .spectral = item->spectral};
        break;
      default:
        return false;
    }
    if (!use_consumable(&c)) return false;
  } else {
    if (!add_item_to_player(item)) return false;
  }

  cvector_erase(state.game.shop.items, index);
  return true;
}

void buy_item(bool should_use) {
  uint8_t item_index = state.navigation.hovered;
  NavigationSection section = get_current_section();

  // Prevent buying items if navigation cursor is outside buyable items section
  if (section != NAVIGATION_SHOP_VOUCHER && section != NAVIGATION_SHOP_BOOSTER_PACKS &&
      section != NAVIGATION_SHOP_ITEMS)
    return;

  uint8_t price = get_item_price(section, item_index);
  if (state.game.money < price) return;

  state.game.money -= price;
  bool was_bought = true;

  switch (section) {
    case NAVIGATION_SHOP_ITEMS:
      was_bought = buy_shop_item(item_index, should_use);
      break;
    case NAVIGATION_SHOP_BOOSTER_PACKS:
      was_bought = buy_booster_pack(item_index);
      break;
    case NAVIGATION_SHOP_VOUCHER:
      was_bought = buy_voucher(item_index);
      break;
    default:
      break;
  }

  if (!was_bought) {
    state.game.money += price;
    return;
  }
  if (state.stage == STAGE_SHOP) set_nav_hovered(state.navigation.hovered);
}

void sell_shop_item() {
  NavigationSection section = get_current_section();
  uint8_t item_index = state.navigation.hovered;
  uint8_t is_consumable = section == NAVIGATION_CONSUMABLES;

  if (get_nav_section_size(section) == 0) return;

  ShopItem item;

  if (is_consumable) {
    Consumable consumable = state.game.consumables.items[item_index];
    switch (consumable.type) {
      case CONSUMABLE_PLANET:
        item.type = SHOP_ITEM_PLANET;
        item.planet = consumable.planet;
        break;
      case CONSUMABLE_TAROT:
        item.type = SHOP_ITEM_TAROT;
        item.tarot = consumable.tarot;
        break;
      case CONSUMABLE_SPECTRAL:
        item.type = SHOP_ITEM_SPECTRAL;
        item.spectral = consumable.spectral;
        break;
    }

    cvector_erase(state.game.consumables.items, item_index);
  } else {
    item.type = SHOP_ITEM_JOKER;
    item.joker = state.game.jokers.cards[item_index];
    cvector_erase(state.game.jokers.cards, item_index);

    if (state.stage == STAGE_GAME && state.game.current_blind->is_active &&
        state.game.current_blind->type == BLIND_VERDANT_LEAF)
      disable_boss_blind();
  }

  state.game.money += get_shop_item_sell_price(&item);
  set_nav_hovered(item_index);
}

#define FILTER_AVAILABLE_BOOSTER_PACK(expected_type, item_type)                                                     \
  do {                                                                                                              \
    cvector_for_each(                                                                                               \
        state.game.booster_pack.content, BoosterPackContent,                                                        \
        item) if (state.game.booster_pack.item.type == expected_type && item->item_type == item_type) return false; \
    return true;                                                                                                    \
  } while (0);
bool filter_available_tarot_booster_pack(uint8_t tarot) { FILTER_AVAILABLE_BOOSTER_PACK(BOOSTER_PACK_ARCANA, tarot); }
bool filter_available_planet_booster_pack(uint8_t planet) {
  if (is_planet_card_locked(planet)) return false;
  FILTER_AVAILABLE_BOOSTER_PACK(BOOSTER_PACK_CELESTIAL, planet);
}
bool filter_available_spectral_booster_pack(uint8_t spectral) {
  FILTER_AVAILABLE_BOOSTER_PACK(BOOSTER_PACK_SPECTRAL, spectral);
}
void open_booster_pack(BoosterPackItem *booster_pack) {
  cvector_clear(state.game.booster_pack.content);
  state.game.booster_pack.item = *booster_pack;
  state.game.booster_pack.uses = booster_pack->size == BOOSTER_PACK_MEGA ? 2 : 1;

  change_stage(STAGE_BOOSTER_PACK);

  shuffle_deck();
  fill_hand();
  sort_hand();

  for (uint8_t i = 0; i < get_booster_pack_items_count(booster_pack); i++) {
    BoosterPackContent content = {0};

    switch (booster_pack->type) {
      case BOOSTER_PACK_STANDARD:
        content.card = random_card();
        break;
      case BOOSTER_PACK_BUFFON:
        content.joker = random_available_joker();
        break;
      case BOOSTER_PACK_CELESTIAL:
        if (state.game.vouchers & VOUCHER_TELESCOPE && i == 0) {
          PokerHand most_played = get_most_played_poker_hand();
          content.planet = ffs(most_played) - 1;
        } else {
          content.planet = random_filtered_range_pick(0, 11, filter_available_planet_booster_pack);
        }
        break;
      case BOOSTER_PACK_ARCANA:
        content.tarot = random_filtered_range_pick(0, 21, filter_available_tarot_booster_pack);
        break;
      case BOOSTER_PACK_SPECTRAL:
        content.spectral = random_filtered_range_pick(0, 15, filter_available_spectral_booster_pack);
        if (random_chance(3, 100))
          content.spectral = SPECTRAL_SOUL;
        else if (random_chance(3, 100))
          content.spectral = SPECTRAL_BLACK_HOLE;
        break;
    }

    cvector_push_back(state.game.booster_pack.content, content);
  }
}

void close_booster_pack() {
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck, state.game.deck);

  change_stage(state.prev_stage);
  if (state.stage == STAGE_SELECT_BLIND) trigger_immediate_tags();
}

void select_booster_pack_item() {
  BoosterPackContent *content = &state.game.booster_pack.content[state.navigation.hovered];

  uint8_t was_used = 1;
  switch (state.game.booster_pack.item.type) {
    case BOOSTER_PACK_STANDARD:
      was_used = add_item_to_player(&(ShopItem){.type = SHOP_ITEM_CARD, .card = content->card});
      break;
    case BOOSTER_PACK_BUFFON:
      was_used = add_item_to_player(&(ShopItem){.type = SHOP_ITEM_JOKER, .joker = content->joker});
      break;

    case BOOSTER_PACK_CELESTIAL:
      was_used = use_consumable(&(Consumable){.type = CONSUMABLE_PLANET, .planet = content->planet});
      break;
    case BOOSTER_PACK_ARCANA:
      was_used = use_consumable(&(Consumable){.type = CONSUMABLE_TAROT, .tarot = content->tarot});
      break;
    case BOOSTER_PACK_SPECTRAL:
      was_used = use_consumable(&(Consumable){.type = CONSUMABLE_SPECTRAL, .spectral = content->spectral});
      break;
  }

  if (was_used == 0) return;

  state.game.booster_pack.uses--;
  cvector_erase(state.game.booster_pack.content, state.navigation.hovered);
  set_nav_hovered(state.navigation.hovered);

  if (state.game.booster_pack.uses == 0) close_booster_pack();
}

void skip_booster_pack() { close_booster_pack(); }

#define FILTER_AVAILABLE_SHOP(expected_type, item_type)                                                   \
  do {                                                                                                    \
    cvector_for_each(state.game.shop.items, ShopItem,                                                     \
                     item) if (item->type == expected_type && item->item_type == item_type) return false; \
    return true;                                                                                          \
  } while (0);
bool filter_available_tarot_shop(uint8_t tarot) { FILTER_AVAILABLE_SHOP(SHOP_ITEM_TAROT, tarot); }
bool filter_available_planet_shop(uint8_t planet) {
  if (is_planet_card_locked(planet)) return false;
  FILTER_AVAILABLE_SHOP(SHOP_ITEM_PLANET, planet);
}
bool filter_available_spectral_shop(uint8_t spectral) { FILTER_AVAILABLE_SHOP(SHOP_ITEM_SPECTRAL, spectral); }

void fill_shop_items() {
  // Card, Tarot, Planet, Joker, Spectral
  uint16_t shop_item_weights[5] = {0, 40, 40, 200, 0};

  if (state.game.vouchers & VOUCHER_MAGIC_TRICK) shop_item_weights[0] = 40;

  if (state.game.vouchers & VOUCHER_TAROT_TYCOON)
    shop_item_weights[1] = 320;
  else if (state.game.vouchers & VOUCHER_TAROT_MERCHANT)
    shop_item_weights[1] = 96;

  if (state.game.vouchers & VOUCHER_PLANET_TYCOON)
    shop_item_weights[2] = 320;
  else if (state.game.vouchers & VOUCHER_PLANET_MERCHANT)
    shop_item_weights[2] = 96;

  if (state.game.deck_type == DECK_GHOST) shop_item_weights[4] = 20;

  while (cvector_size(state.game.shop.items) < state.game.shop.size) {
    ShopItemType type = random_weighted(shop_item_weights, 5);
    ShopItem item = {.type = type};

    switch (type) {
      case SHOP_ITEM_CARD:
        item.card = random_shop_card();
        break;
      case SHOP_ITEM_TAROT:
        item.tarot = random_filtered_range_pick(0, 21, filter_available_tarot_shop);
        break;
      case SHOP_ITEM_PLANET:
        item.planet = random_filtered_range_pick(0, 11, filter_available_planet_shop);
        break;
      case SHOP_ITEM_JOKER:
        item.joker = random_available_joker();
        break;
      case SHOP_ITEM_SPECTRAL:
        item.spectral = random_filtered_range_pick(0, 15, filter_available_spectral_shop);
        break;
    }

    cvector_push_back(state.game.shop.items, item);
  }
}

uint8_t get_reroll_price() {
  cvector_for_each(state.game.tags, Tag, tag) if (*tag == TAG_D6) return state.game.shop.reroll_count;

  uint8_t base_price = (state.game.vouchers & VOUCHER_REROLL_GLUT)      ? 1
                       : (state.game.vouchers & VOUCHER_REROLL_SURPLUS) ? 3
                                                                        : 5;
  return base_price + state.game.shop.reroll_count;
}

void reroll_shop_items() {
  uint8_t price = get_reroll_price();
  if (state.game.money < price) return;

  state.game.shop.reroll_count++;
  state.game.money -= price;

  cvector_clear(state.game.shop.items);
  fill_shop_items();
}

bool filter_available_vouchers(uint8_t i) {
  if (state.game.vouchers & (1 << i)) return false;
  cvector_for_each(state.game.shop.vouchers, Voucher, voucher) if (*voucher == 1 << i) return false;
  if (i < 16 || state.game.vouchers & (1 << (i - 16))) return true;

  return false;
}

void restock_shop() {
  cvector_clear(state.game.shop.items);
  cvector_clear(state.game.shop.booster_packs);
  while (cvector_size(state.game.shop.vouchers) > 1) cvector_erase(state.game.shop.vouchers, 0);

  uint8_t is_ante_first_shop = state.game.current_blind->type == BLIND_SMALL ||
                               (!state.game.blinds[0].is_active &&
                                (state.game.current_blind->type == BLIND_BIG ||
                                 (!state.game.blinds[1].is_active && state.game.current_blind->type > BLIND_BIG)));

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    Tag tag = state.game.tags[i];
    if (tag != TAG_UNCOMMON && tag != TAG_RARE) continue;

    // TODO Fix adding duplicates and wrong rarity jokers when rng utilities will be added
    ShopItem joker = (ShopItem){.type = SHOP_ITEM_JOKER, .joker = JOKERS[random_max_value(JOKER_COUNT - 1)]};
    cvector_push_back(state.game.shop.items, joker);

    cvector_erase(state.game.tags, i);
    i--;
  }

  fill_shop_items();

  // First visit to the Shop in a run guarantees one normal Buffoon Pack
  if (state.game.round == 1) {
    BoosterPackItem booster_pack = {.type = BOOSTER_PACK_BUFFON, .size = BOOSTER_PACK_NORMAL};
    cvector_push_back(state.game.shop.booster_packs, booster_pack);
  }

  // Standard, Arcana, Celestial, Buffoon, Spectral
  // Normal, Jumbo, Mega
  // Standard Normal, Standard Jumbo, Standard Mega, Arcana Normal,...
  uint16_t booster_pack_weights[5 * 3] = {400, 200, 50, 400, 200, 50, 400, 200, 50, 120, 60, 15, 60, 30, 7};
  for (uint8_t i = 0; i < (state.game.round == 1 ? 1 : 2); i++) {
    uint8_t random_value = random_weighted(booster_pack_weights, 15);
    BoosterPackItem booster_pack = {.type = random_value / 3, .size = random_value % 3};
    cvector_push_back(state.game.shop.booster_packs, booster_pack);
  }

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == TAG_VOUCHER) {
      cvector_insert(state.game.shop.vouchers, 0, 1 << random_filtered_range_pick(0, 31, filter_available_vouchers));
      cvector_erase(state.game.tags, i);
      i--;
    }

    cvector_for_each(state.game.shop.items, ShopItem, shop_item) {
      if (shop_item->type != SHOP_ITEM_JOKER || shop_item->joker.edition != EDITION_BASE) continue;

      switch (state.game.tags[i]) {
        case TAG_NEGATIVE:
          shop_item->joker.edition = EDITION_NEGATIVE;
          break;
        case TAG_FOIL:
          shop_item->joker.edition = EDITION_FOIL;
          break;
        case TAG_HOLOGRAPHIC:
          shop_item->joker.edition = EDITION_HOLOGRAPHIC;
          break;
        case TAG_POLYCHROME:
          shop_item->joker.edition = EDITION_POLYCHROME;
          break;
        default:
          continue;
      }

      // TODO Fix modifying base price as it modifies sell price as well, probably adding sell price property to shop
      // items will be a way to go
      shop_item->joker.base_price = 0;
      cvector_erase(state.game.tags, i);
      i--;
    }
  }

  if (is_ante_first_shop || state.game.round == 1)
    cvector_back(state.game.shop.vouchers) = 1 << random_filtered_range_pick(0, 31, filter_available_vouchers);
}

void erase_first_tag_occurance(Tag tag) {
  for (uint8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == tag) {
      cvector_erase(state.game.tags, i);
      return;
    }
  }
}

void exit_shop() {
  erase_first_tag_occurance(TAG_COUPON);
  erase_first_tag_occurance(TAG_D6);

  state.game.shop.reroll_count = 0;

  change_stage(STAGE_SELECT_BLIND);
}

void select_blind() {
  state.game.round++;

  cvector_for_each(state.game.tags, Tag, tag) {
    if (*tag == TAG_JUGGLE) state.game.hand.size += 3;
  }

  if (state.game.current_blind->type > BLIND_BIG) enable_boss_blind();

  shuffle_deck();
  fill_hand();
  sort_hand();

  if (state.game.current_blind->type == BLIND_HOUSE)
    cvector_for_each(state.game.hand.cards, Card, card) card->status |= CARD_STATUS_FACE_DOWN;
  else if (state.game.current_blind->type == BLIND_CERULEAN_BELL)
    force_card_select(random_vector_index(state.game.hand.cards));

  change_stage(STAGE_GAME);
}

void skip_blind() {
  if (state.game.current_blind->type > BLIND_BIG) return;

  state.game.current_blind->is_active = 0;
  cvector_push_back(state.game.tags, state.game.current_blind->tag);
  for (int8_t i = cvector_size(state.game.tags) - 2; i >= 0; i--) {
    if (state.game.tags[i] != TAG_DOUBLE) break;
    state.game.tags[i] = state.game.current_blind->tag;
  }
  state.game.current_blind++;

  trigger_immediate_tags();
  set_nav_hovered(0);
}

void trigger_immediate_tags() {
  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    uint8_t should_stop = 0;
    switch (state.game.tags[i]) {
      case TAG_BOSS:
        roll_boss_blind();
        break;
      case TAG_STANDARD:
        open_booster_pack(&(BoosterPackItem){.type = BOOSTER_PACK_STANDARD, BOOSTER_PACK_MEGA});
        should_stop = 1;
        break;
      case TAG_CHARM:
        open_booster_pack(&(BoosterPackItem){.type = BOOSTER_PACK_ARCANA, BOOSTER_PACK_MEGA});
        should_stop = 1;
        break;
      case TAG_METEOR:
        open_booster_pack(&(BoosterPackItem){.type = BOOSTER_PACK_CELESTIAL, BOOSTER_PACK_MEGA});
        should_stop = 1;
        break;
      case TAG_BUFFOON:
        open_booster_pack(&(BoosterPackItem){.type = BOOSTER_PACK_BUFFON, BOOSTER_PACK_MEGA});
        should_stop = 1;
        break;
      case TAG_HANDY:
        state.game.money += state.game.stats.hands.total - state.game.stats.hands.remaining;
        break;
      case TAG_GARBAGE:
        state.game.money += state.game.stats.discards.remaining;
        break;
      case TAG_ETHEREAL:
        open_booster_pack(&(BoosterPackItem){.type = BOOSTER_PACK_SPECTRAL, BOOSTER_PACK_NORMAL});
        should_stop = 1;
        break;
      case TAG_TOPUP:
        for (uint8_t i = 0; i < 2; i++)
          add_item_to_player(
              &(ShopItem){.type = SHOP_ITEM_JOKER, .joker = random_available_joker_by_rarity(RARITY_COMMON)});
        break;
      case TAG_SPEED: {
        uint8_t total_rounds = (state.game.ante - 1) * 3 + (state.game.current_blind->type == BLIND_BIG ? 1 : 2);
        if (state.game.vouchers & VOUCHER_HIEROGLYPH) total_rounds += 3;
        if (state.game.vouchers & VOUCHER_PTEROGLYPH) total_rounds += 3;
        state.game.money += 5 * (total_rounds - state.game.round);
        break;
      }
      case TAG_ORBITAL:
        state.game.poker_hands[random_filtered_range_pick(0, 11, filter_locked_planet_cards)].level += 3;
        break;
      case TAG_ECONOMY:
        if (state.game.money > 0) state.game.money += state.game.money > 40 ? 40 : state.game.money;
        break;

      default:
        continue;
    }

    cvector_erase(state.game.tags, i);
    i--;

    if (should_stop) break;
  }
}

PokerHand get_most_played_poker_hand() {
  uint16_t max_played = 0;
  PokerHand max_hand = HAND_HIGH_CARD;

  for (int8_t i = 11; i >= 0; i--) {
    if (state.game.poker_hands[i].played > max_played) {
      max_played = state.game.poker_hands[i].played;
      max_hand = 1 << i;
    }
  }

  return max_hand;
}

uint8_t get_blind_min_ante(BlindType blind) {
  switch (blind) {
    case BLIND_SMALL:
    case BLIND_BIG:
    case BLIND_HOOK:
    case BLIND_CLUB:
    case BLIND_PSYCHIC:
    case BLIND_GOAD:
    case BLIND_WINDOW:
    case BLIND_MANACLE:
    case BLIND_PILLAR:
    case BLIND_HEAD:
      return 1;

    case BLIND_HOUSE:
    case BLIND_WALL:
    case BLIND_WHEEL:
    case BLIND_ARM:
    case BLIND_FISH:
    case BLIND_WATER:
    case BLIND_MOUTH:
    case BLIND_NEEDLE:
    case BLIND_FLINT:
    case BLIND_MARK:
      return 2;

    case BLIND_EYE:
    case BLIND_TOOTH:
      return 3;

    case BLIND_PLANT:
      return 4;

    case BLIND_SERPENT:
      return 5;

    case BLIND_OX:
      return 6;

    case BLIND_AMBER_ACORN:
    case BLIND_VERDANT_LEAF:
    case BLIND_VIOLET_VESSEL:
    case BLIND_CRIMSON_HEART:
    case BLIND_CERULEAN_BELL:
      return 8;
  }
}

bool filter_defeated_boss_blinds(uint8_t i) { return !(state.game.defeated_boss_blinds & 1 << i); }
bool filter_available_boss_blinds(uint8_t i) {
  return (state.game.ante <= 0 ? 1 : state.game.ante) >= get_blind_min_ante(i) && filter_defeated_boss_blinds(i);
}

void roll_boss_blind() {
  uint8_t available_boss_blinds = 0;

  if (state.game.ante > 0 && state.game.ante % 8 == 0) {
    state.game.blinds[2].type =
        random_filtered_range_pick(BLIND_AMBER_ACORN, BLIND_CERULEAN_BELL, filter_defeated_boss_blinds);
    return;
  }

  state.game.blinds[2].type = random_filtered_range_pick(BLIND_HOOK, BLIND_MARK, filter_available_boss_blinds);
}

void trigger_reroll_boss_voucher() {
  if (!(state.game.vouchers & VOUCHER_DIRECTORS_CUT)) return;
  if (!(state.game.vouchers & VOUCHER_RETCON) && state.game.has_rerolled_boss) return;

  if (state.game.money >= 10) {
    state.game.money -= 10;
    roll_boss_blind();
  }

  state.game.has_rerolled_boss = 1;
}

#define DEBUFF_CARDS_IF(COND)                                                                           \
  do {                                                                                                  \
    cvector_for_each(state.game.deck, Card, card) if (COND) card->status |= CARD_STATUS_DEBUFFED;       \
    cvector_for_each(state.game.hand.cards, Card, card) if (COND) card->status |= CARD_STATUS_DEBUFFED; \
  } while (0)

void enable_boss_blind() {
  switch (state.game.current_blind->type) {
    case BLIND_CLUB:
      DEBUFF_CARDS_IF(is_suit(card, SUIT_CLUBS));
      break;
    case BLIND_GOAD:
      DEBUFF_CARDS_IF(is_suit(card, SUIT_SPADES));
      break;
    case BLIND_WATER:
      state.game.discards.remaining = 0;
      break;
    case BLIND_WINDOW:
      DEBUFF_CARDS_IF(is_suit(card, SUIT_DIAMONDS));
      break;
    case BLIND_MANACLE:
      state.game.hand.size--;
      break;
    case BLIND_PLANT:
      DEBUFF_CARDS_IF(is_face_card(card));
      break;
    case BLIND_HEAD:
      DEBUFF_CARDS_IF(is_suit(card, SUIT_HEARTS));
      break;
    case BLIND_PILLAR:
      DEBUFF_CARDS_IF(card->was_played);
      break;
    case BLIND_NEEDLE:
      state.game.hands.remaining = 1;
      break;

    case BLIND_AMBER_ACORN:
      for (uint8_t i = cvector_size(state.game.jokers.cards) - 1; i > 0; i--) {
        uint8_t j = rand() % (i + 1);
        Joker temp = state.game.jokers.cards[i];
        state.game.jokers.cards[i] = state.game.jokers.cards[j];
        state.game.jokers.cards[j] = temp;
      }
      cvector_for_each(state.game.jokers.cards, Joker, joker) joker->status |= CARD_STATUS_FACE_DOWN;
      break;
    case BLIND_VERDANT_LEAF:
      DEBUFF_CARDS_IF(1);
      break;
    case BLIND_CRIMSON_HEART:
      if (cvector_size(state.game.jokers.cards) > 0)
        random_vector_item(state.game.jokers.cards).status |= CARD_STATUS_DEBUFFED;
      break;
    default:
      break;
  }
}

void disable_boss_blind() {
  state.game.current_blind->is_active = 0;
  switch (state.game.current_blind->type) {
    case BLIND_WATER:
      state.game.discards.remaining = state.game.discards.total;
      break;
    case BLIND_MANACLE:
      state.game.hand.size++;
      break;
    case BLIND_NEEDLE:
      state.game.hands.remaining = state.game.hands.total;
      break;
    default:
      break;
  }

  cvector_for_each(state.game.hand.cards, Card, card) card->status = CARD_STATUS_NORMAL;
  cvector_for_each(state.game.deck, Card, card) card->status = CARD_STATUS_NORMAL;
  cvector_for_each(state.game.jokers.cards, Joker, joker) joker->status = CARD_STATUS_NORMAL;
}
