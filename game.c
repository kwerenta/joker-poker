#include "game.h"

#include <cvector.h>
#include <math.h>
#include <stdint.h>

#include "content/spectral.h"
#include "content/tarot.h"
#include "debug.h"
#include "state.h"

void game_init() {
  // Generate standard deck of 52 cards
  cvector_reserve(state.game.full_deck, 52);
  for (uint8_t i = 0; i < 52; i++) {
    cvector_push_back(state.game.full_deck, create_card(i % 4, i % 13, EDITION_BASE, ENHANCEMENT_NONE, SEAL_NONE));
  }

  cvector_copy(state.game.full_deck, state.game.deck);

  shuffle_deck();

  state.game.hand.size = 7;
  cvector_reserve(state.game.hand.cards, state.game.hand.size);
  fill_hand();
  sort_hand();

  state.game.money = 4;

  state.game.score = 0;

  state.game.ante = 1;
  state.game.blind = 0;
  state.game.round = 0;

  state.game.hands.total = 4;
  state.game.hands.remaining = 4;

  state.game.discards.total = 2;
  state.game.discards.remaining = 2;

  state.game.jokers.size = 5;
  state.game.consumables.size = 2;

  state.game.shop.size = 2;
  cvector_reserve(state.game.shop.booster_packs, 2);

  change_stage(STAGE_GAME);

  state.game.fool_last_used.was_used = 0;

  memset(state.game.poker_hands, 0, 12 * sizeof(PokerHandStats));

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

  log_message(LOG_INFO, "Game has been destroyed.");
}

uint8_t compare_cards(Card *a, Card *b) {
  return a->chips == b->chips && a->enhancement == b->enhancement && a->edition == b->edition && a->rank == b->rank &&
         a->suit == b->suit && a->seal == b->seal;
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
}

void fill_hand() {
  while (cvector_size(state.game.hand.cards) < state.game.hand.size) draw_card();
}

void play_hand() {
  if (state.game.hands.remaining == 0 || state.game.selected_hand.count == 0) return;

  update_scoring_hand();
  get_poker_hand_stats(state.game.selected_hand.hand_union)->played++;

  for (uint8_t i = 0; i < 5; i++) {
    Card *card = state.game.selected_hand.scoring_cards[i];
    if (card == NULL) continue;

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

        if (rand() % 4 == 0) {
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
        if (rand() % 5 == 0) state.game.selected_hand.score_pair.mult += 20;
        if (rand() % 15 == 0) state.game.money += 20;
        break;
    }

    if (card->seal == SEAL_GOLD) state.game.money += 3;
  }

  cvector_for_each(state.game.jokers.cards, Joker, joker) {
    if (joker->activation_type == ACTIVATION_INDEPENDENT) joker->activate();

    update_scoring_edition(joker->edition);
  }

  remove_selected_cards();
  state.game.hands.remaining--;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->enhancement == ENHANCEMENT_STEEL) state.game.selected_hand.score_pair.mult *= 1.5;
  }

  if (state.game.vouchers & VOUCHER_OBSERVATORY) {
    cvector_for_each(state.game.consumables.items, Consumable, consumable) {
      if (consumable->type == CONSUMABLE_PLANET && (1 << consumable->planet) == state.game.selected_hand.hand_union) {
        state.game.selected_hand.score_pair.mult *= 1.5;
      }
    }
  }

  state.game.score += state.game.selected_hand.score_pair.chips * state.game.selected_hand.score_pair.mult;

  double required_score = get_required_score(state.game.ante, state.game.blind);

  if (state.game.score >= required_score) {
    cvector_for_each(state.game.hand.cards, Card, card) {
      if (card->enhancement == ENHANCEMENT_GOLD) state.game.money += 3;
      if (card->seal == SEAL_BLUE)
        add_item_to_player(
            &(ShopItem){.type = SHOP_ITEM_PLANET, .planet = ffs(state.game.selected_hand.hand_union) - 1});
    }

    change_stage(STAGE_CASH_OUT);
  } else if (state.game.hands.remaining == 0) {
    change_stage(STAGE_GAME_OVER);
  } else {
    fill_hand();
    sort_hand();
  }
}

void get_cash_out() {
  state.game.money += get_interest_money() + get_hands_money() + get_blind_money(state.game.blind);

  state.game.score = 0;

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

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected == 1 && card->seal == SEAL_PURPLE)
      add_item_to_player(&(ShopItem){.type = SHOP_ITEM_TAROT, .tarot = rand() % 22});
  }

  state.game.discards.remaining--;
  remove_selected_cards();
  fill_hand();
  sort_hand();
}

void remove_selected_cards() {
  uint8_t i = 0;
  while (i < cvector_size(state.game.hand.cards)) {
    if (state.game.hand.cards[i].selected == 1) {
      cvector_erase(state.game.hand.cards, i);
      continue;
    }

    i++;
  }

  state.game.selected_hand.count = 0;
}

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

void deselect_all_cards() {
  cvector_for_each(state.game.hand.cards, Card, card) { card->selected = 0; }
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

double get_required_score(uint8_t ante, uint8_t blind) {
  return get_ante_base_score(ante) * (blind == 0 ? 1 : blind == 1 ? 1.5 : 2);
}

uint8_t get_blind_money(uint8_t blind) { return blind == 0 ? 3 : blind == 1 ? 4 : 5; }
uint8_t get_hands_money() { return state.game.hands.remaining; }

uint8_t get_interest_money() {
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

uint8_t get_booster_pack_price(BoosterPackItem *booster_pack) { return apply_sale(4 + booster_pack->size * 2); }
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

void buy_shop_item() {
  uint8_t item_index = state.navigation.hovered;
  NavigationSection section = get_current_section();
  uint8_t is_booster_pack = section == NAVIGATION_SHOP_BOOSTER_PACKS;
  uint8_t is_voucher = section == NAVIGATION_SHOP_VOUCHER;

  // Prevent buying items if navigation cursor is outside buyable items section
  if (!is_booster_pack && !is_voucher && section != NAVIGATION_SHOP_ITEMS) return;

  uint8_t price = is_booster_pack ? get_booster_pack_price(&state.game.shop.booster_packs[item_index])
                  : is_voucher    ? get_voucher_price(state.game.shop.voucher)
                                  : get_shop_item_price(&state.game.shop.items[item_index]);

  if (state.game.money < price) return;

  if (is_booster_pack) {
    open_booster_pack(&state.game.shop.booster_packs[item_index]);
    cvector_erase(state.game.shop.booster_packs, item_index);
  } else if (is_voucher) {
    add_voucher_to_player(state.game.shop.voucher);
    state.game.shop.voucher = 0;
  } else {
    if (add_item_to_player(&state.game.shop.items[item_index]) == 0) return;
    cvector_erase(state.game.shop.items, item_index);
  }

  state.game.money -= price;

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
  }

  state.game.money += get_shop_item_sell_price(&item);
  set_nav_hovered(item_index);
}

void open_booster_pack(BoosterPackItem *booster_pack) {
  cvector_clear(state.game.booster_pack.content);
  state.game.booster_pack.item = *booster_pack;
  state.game.booster_pack.uses = booster_pack->size == BOOSTER_PACK_MEGA ? 2 : 1;

  shuffle_deck();
  fill_hand();
  sort_hand();

  change_stage(STAGE_BOOSTER_PACK);

  for (uint8_t i = 0; i < get_booster_pack_items_count(booster_pack); i++) {
    BoosterPackContent content = {0};

    switch (booster_pack->type) {
      case BOOSTER_PACK_STANDARD:
        content.card = create_card(rand() % 4, rand() % 13, EDITION_BASE, ENHANCEMENT_NONE, SEAL_NONE);
        break;
      case BOOSTER_PACK_BUFFON:
        content.joker = JOKERS[rand() % JOKER_COUNT];
        break;
      case BOOSTER_PACK_CELESTIAL:
        if (state.game.vouchers & VOUCHER_TELESCOPE && i == 0) {
          uint16_t max_played = 0;

          for (int8_t j = 11; j >= 0; j--) {
            if (state.game.poker_hands[j].played >= max_played) {
              max_played = state.game.poker_hands[j].played;
              content.planet = j;
            }
          }
        } else {
          content.planet = rand() % 12;
        }
        break;
      case BOOSTER_PACK_ARCANA:
        content.tarot = rand() % 22;
        break;
      case BOOSTER_PACK_SPECTRAL:
        content.spectral = rand() % 18;
        break;
    }

    cvector_push_back(state.game.booster_pack.content, content);
  }
}

void close_booster_pack() {
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck, state.game.deck);

  change_stage(STAGE_SHOP);
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

void fill_shop_items() {
  ShopItem item = {0};

  while (cvector_size(state.game.shop.items) < state.game.shop.size) {
    switch (rand() % 4) {
      case 0:
        item = (ShopItem){.type = SHOP_ITEM_CARD,
                          .card = create_card(rand() % 4, rand() % 13, EDITION_BASE, ENHANCEMENT_NONE, SEAL_NONE)};
        break;
      case 1:
        item = (ShopItem){.type = SHOP_ITEM_JOKER, .joker = JOKERS[rand() % JOKER_COUNT]};
        break;
      case 2:
        item = (ShopItem){.type = SHOP_ITEM_PLANET, .planet = rand() % 12};
        break;
      case 3:
        item = (ShopItem){.type = SHOP_ITEM_TAROT, .tarot = rand() % 22};
        break;
    }

    cvector_push_back(state.game.shop.items, item);
  }
}

void restock_shop() {
  cvector_clear(state.game.shop.items);
  cvector_clear(state.game.shop.booster_packs);

  fill_shop_items();

  for (uint8_t i = 0; i < 2; i++) {
    BoosterPackItem booster_pack = {.type = rand() % 5, .size = rand() % 3};
    cvector_push_back(state.game.shop.booster_packs, booster_pack);
  }

  if (state.game.blind % 3 == 0) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 32; i++) {
      if (state.game.vouchers & (1 << i)) continue;

      if (i < 16) {
        count++;
        continue;
      }
      if (state.game.vouchers & (1 << (i - 16))) count++;
    }

    uint8_t voucher_index = rand() % count;

    for (uint8_t i = 0; i < 32; i++) {
      if (state.game.vouchers & (1 << i)) continue;

      if (voucher_index == 0) state.game.shop.voucher = 1 << i;
      voucher_index--;
    }
  }
}

void exit_shop() {
  state.game.round++;
  state.game.blind++;

  if (state.game.blind > 2) {
    state.game.blind = 0;
    state.game.ante++;
  }

  shuffle_deck();
  fill_hand();
  sort_hand();

  change_stage(STAGE_GAME);
}
