#include "game.h"

#include <cvector.h>
#include <stdint.h>

#include "debug.h"
#include "state.h"

void game_init() {
  // Generate standard deck of 52 cards
  cvector_reserve(state.game.full_deck, 52);
  for (uint8_t i = 0; i < 52; i++) {
    cvector_push_back(state.game.full_deck, create_card(i % 4, i % 13, EDITION_BASE, ENHANCEMENT_NONE));
  }

  cvector_copy(state.game.full_deck, state.game.deck);

  shuffle_deck();

  state.game.hand.size = 7;
  cvector_reserve(state.game.hand.cards, state.game.hand.size);
  fill_hand();

  state.game.money = 4;

  state.game.score = 0;

  state.game.ante = 1;
  state.game.blind = 0;
  state.game.round = 0;

  state.game.hands = 4;
  state.game.discards = 2;

  state.game.jokers.size = 5;
  state.game.consumables.size = 2;

  state.game.shop.size = 2;
  cvector_reserve(state.game.shop.booster_packs, 2);

  state.navigation.hovered = 0;
  state.navigation.section = NAVIGATION_HAND;

  memset(state.game.poker_hands, 0, 12 * sizeof(uint8_t));

  state.stage = STAGE_GAME;

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

Card create_card(Suit suit, Rank rank, Edition edition, Enhancement enhancement) {
  uint16_t chips = rank == RANK_ACE ? 11 : rank + 1;
  if (rank != RANK_ACE && chips > 10) chips = 10;

  return (Card){
      .suit = suit, .rank = rank, .chips = chips, .edition = edition, .enhancement = enhancement, .selected = 0};
}

void draw_card() {
  cvector_push_back(state.game.hand.cards, cvector_back(state.game.deck));
  cvector_pop_back(state.game.deck);
}

void fill_hand() {
  while (cvector_size(state.game.hand.cards) < state.game.hand.size) draw_card();
}

void play_hand() {
  if (state.game.hands == 0 || state.game.selected_hand.count == 0) return;

  update_scoring_hand();

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
            if (card->chips == other->chips && card->enhancement == other->enhancement &&
                card->edition == other->edition && card->rank == other->rank && card->suit == other->suit) {
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
  }

  cvector_for_each(state.game.jokers.cards, Joker, joker) {
    if (joker->activation_type == ACTIVATION_INDEPENDENT) joker->activate();

    update_scoring_edition(joker->edition);
  }

  remove_selected_cards();
  state.game.hands--;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->enhancement == ENHANCEMENT_STEEL) state.game.selected_hand.score_pair.mult *= 1.5;
  }

  state.game.score += state.game.selected_hand.score_pair.chips * state.game.selected_hand.score_pair.mult;

  double required_score = get_required_score(state.game.ante, state.game.blind);

  if (state.game.score >= required_score) {
    cvector_for_each(state.game.hand.cards, Card, card) {
      if (card->enhancement == ENHANCEMENT_GOLD) state.game.money += 3;
    }

    change_stage(STAGE_CASH_OUT);
  } else if (state.game.hands == 0) {
    state.stage = STAGE_GAME_OVER;
  } else {
    fill_hand();
  }
}

void get_cash_out() {
  state.game.money += get_interest_money() + get_hands_money() + get_blind_money(state.game.blind);

  state.game.score = 0;

  // Reset hand and deck for new blind
  state.game.hands = 4;
  state.game.discards = 2;
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
  if (state.game.selected_hand.count == 0 || state.game.discards == 0) return;

  state.game.discards--;
  remove_selected_cards();
  fill_hand();
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

void sort_hand(uint8_t by_suit) {
  int (*comparator)(const void *a, const void *b) = compare_by_rank;
  if (by_suit == 1) comparator = compare_by_suit;

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

ScorePair get_poker_hand_total_score(uint16_t hand_union) {
  ScorePair poker_hand_score = get_poker_hand_base_score(hand_union);
  ScorePair planet_score = get_planet_card_base_score(hand_union);

  uint8_t poker_hand_index = ffs(hand_union) - 1;
  poker_hand_score.chips += planet_score.chips * state.game.poker_hands[poker_hand_index];
  poker_hand_score.mult += planet_score.mult * state.game.poker_hands[poker_hand_index];

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
uint8_t get_hands_money() { return state.game.hands; }

uint8_t get_interest_money() {
  // TODO Add max interest cap
  return state.game.money / 5;
}

// If consumable is NULL then hovered item from consumables section will be used
void use_consumable(Consumable *consumable_to_use) {
  Consumable *consumable = consumable_to_use;

  if (consumable_to_use == NULL) {
    if (cvector_size(state.game.consumables.items) == 0) return;

    consumable = &cvector_at(state.game.consumables.items, state.navigation.hovered);
  }

  switch (consumable->type) {
    case CONSUMABLE_PLANET:
      state.game.poker_hands[consumable->planet] += 1;
      break;
  }

  if (consumable_to_use == NULL) {
    cvector_erase(state.game.consumables.items, state.navigation.hovered);

    if (cvector_size(state.game.consumables.items) >= state.navigation.hovered && state.navigation.hovered > 0)
      state.navigation.hovered--;
  }
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
  }

  return 1;
}

uint8_t get_shop_item_price(ShopItem *item) {
  switch (item->type) {
    case SHOP_ITEM_CARD:
      return 1;
    case SHOP_ITEM_PLANET:
      return 3;
    case SHOP_ITEM_JOKER:
      return item->joker.base_price;
  }
}

uint8_t get_booster_pack_price(BoosterPackItem *booster_pack) { return 4 + booster_pack->size * 2; }
uint8_t get_booster_pack_items_count(BoosterPackItem *booster_pack) {
  uint8_t count = booster_pack->size == BOOSTER_PACK_NORMAL ? 3 : 5;
  if (booster_pack->type == BOOSTER_PACK_BUFFON) count--;
  return count;
}

void buy_shop_item() {
  uint8_t item_index = state.navigation.hovered;
  uint8_t is_booster_pack = state.navigation.section == NAVIGATION_SHOP_BOOSTER_PACKS;

  uint8_t price = is_booster_pack ? get_booster_pack_price(&state.game.shop.booster_packs[item_index])
                                  : get_shop_item_price(&state.game.shop.items[item_index]);

  if (state.game.money < price) return;

  if (is_booster_pack) {
    open_booster_pack(&state.game.shop.booster_packs[item_index]);
    cvector_erase(state.game.shop.booster_packs, item_index);
  } else {
    if (add_item_to_player(&state.game.shop.items[item_index]) == 0) return;
    cvector_erase(state.game.shop.items, item_index);
  }

  state.game.money -= price;

  if (state.stage == STAGE_SHOP && state.navigation.hovered > 0) state.navigation.hovered--;
}

void open_booster_pack(BoosterPackItem *booster_pack) {
  cvector_clear(state.game.booster_pack.content);
  state.game.booster_pack.item = *booster_pack;
  state.game.booster_pack.uses = booster_pack->size == BOOSTER_PACK_MEGA ? 2 : 1;

  change_stage(STAGE_BOOSTER_PACK);

  for (uint8_t i = 0; i < get_booster_pack_items_count(booster_pack); i++) {
    BoosterPackContent content = {0};

    switch (booster_pack->type) {
      case BOOSTER_PACK_STANDARD:
        content.card = create_card(rand() % 4, rand() % 13, EDITION_BASE, ENHANCEMENT_NONE);
        break;

      case BOOSTER_PACK_BUFFON:
        content.joker = JOKERS[rand() % JOKER_COUNT];
        break;

      case BOOSTER_PACK_CELESTIAL:
        content.planet = rand() % 12;
        break;
    }

    cvector_push_back(state.game.booster_pack.content, content);
  }
}

void select_booster_pack_item() {
  BoosterPackContent *content = &state.game.booster_pack.content[state.navigation.hovered];

  switch (state.game.booster_pack.item.type) {
    case BOOSTER_PACK_STANDARD:
      add_item_to_player(&(ShopItem){.type = SHOP_ITEM_CARD, .card = content->card});
      break;

    case BOOSTER_PACK_BUFFON:
      add_item_to_player(&(ShopItem){.type = SHOP_ITEM_JOKER, .joker = content->joker});
      break;

    case BOOSTER_PACK_CELESTIAL:
      use_consumable(&(Consumable){.type = CONSUMABLE_PLANET, .planet = content->planet});
      break;
  }

  state.game.booster_pack.uses--;
  cvector_erase(state.game.booster_pack.content, state.navigation.hovered);
  if (state.navigation.hovered >= cvector_size(state.game.booster_pack.content) - 1) state.navigation.hovered--;

  if (state.game.booster_pack.uses == 0) change_stage(STAGE_SHOP);
}

void skip_booster_pack() { change_stage(STAGE_SHOP); }

void restock_shop() {
  cvector_clear(state.game.shop.items);
  cvector_clear(state.game.shop.booster_packs);

  ShopItem item = {0};

  for (uint8_t i = 0; i < state.game.shop.size; i++) {
    switch (rand() % 3) {
      case 0:
        item = (ShopItem){.type = SHOP_ITEM_CARD,
                          .card = create_card(rand() % 4, rand() % 13, EDITION_BASE, ENHANCEMENT_NONE)};
        break;
      case 1:
        item = (ShopItem){.type = SHOP_ITEM_JOKER, .joker = JOKERS[rand() % JOKER_COUNT]};
        break;
      case 2:
        item = (ShopItem){.type = SHOP_ITEM_PLANET, .planet = rand() % 12};
        break;
    }

    cvector_push_back(state.game.shop.items, item);
  }

  for (uint8_t i = 0; i < 2; i++) {
    BoosterPackItem booster_pack = {.type = rand() % 3, .size = rand() % 3};
    cvector_push_back(state.game.shop.booster_packs, booster_pack);
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

  change_stage(STAGE_GAME);
}
