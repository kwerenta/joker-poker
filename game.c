#include "game.h"

#include <cvector.h>
#include <math.h>
#include <stdint.h>

#include "content/spectral.h"
#include "content/tarot.h"
#include "debug.h"
#include "state.h"

void game_init(Deck deck, Stake stake) {
  state.game.deck_type = deck;
  state.game.stake = stake;

  generate_deck();

  state.game.score = 0;
  state.game.ante = 1;
  state.game.round = 0;

  state.game.blinds[0] = (Blind){.type = BLIND_SMALL, .tag = rand() % 24, .is_active = 1};
  state.game.blinds[1] = (Blind){.type = BLIND_BIG, .tag = rand() % 24, .is_active = 1};
  state.game.blinds[2] = (Blind){.type = rand() % (BLIND_MARK - BLIND_HOOK + 1) + BLIND_HOOK, .is_active = 1};
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
      rank = rand() % 13;
      suit = rand() % 4;
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
      // TODO Add spectral cards to the shop when it will have proper shop item weights
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
  get_poker_hand_stats(state.game.selected_hand.hand_union)->played++;

  for (uint8_t i = 0; i < 5; i++) {
    Card *card = state.game.selected_hand.scoring_cards[i];
    if (card == NULL) continue;

    trigger_scoring_card(card);
    if (card->seal == SEAL_RED) trigger_scoring_card(card);
  }

  cvector_for_each(state.game.jokers.cards, Joker, joker) {
    if (joker->activation_type == ACTIVATION_INDEPENDENT) joker->activate();

    update_scoring_edition(joker->edition);
  }

  remove_selected_cards();
  state.game.hands.remaining--;

  if (state.game.current_blind->type == BLIND_HOOK) {
    for (uint8_t i = 0; i < 2; i++) discard_card(rand() % cvector_size(state.game.hand.cards));
  }

  cvector_for_each(state.game.hand.cards, Card, card) {
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
      trigger_end_of_round_card(card);
      if (card->seal == SEAL_RED) trigger_end_of_round_card(card);
    }

    state.game.stats.hands.total += state.game.hands.total;
    state.game.stats.hands.remaining += state.game.hands.remaining;
    state.game.stats.discards.total += state.game.discards.total;
    state.game.stats.discards.remaining += state.game.discards.remaining;

    change_stage(STAGE_CASH_OUT);
  } else if (state.game.hands.remaining == 0) {
    change_stage(STAGE_GAME_OVER);
  } else {
    fill_hand();
    sort_hand();
  }
}

void cash_out() {
  state.game.money += get_interest_money() + get_hands_money() + get_discards_money() +
                      get_blind_money(state.game.current_blind->type) + get_investment_tag_money();

  state.game.score = 0;

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == TAG_JUGGLE) {
      state.game.hand.size -= 3;
      cvector_erase(state.game.tags, i);
      i--;
    }
  }

  // Reset hand and deck for new blind
  state.game.hands.remaining = state.game.hands.total;
  state.game.discards.remaining = state.game.discards.total;
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck, state.game.deck);

  if (state.game.current_blind->type > BLIND_BIG) {
    state.game.ante++;

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
      if (i != 2) state.game.blinds[i].tag = rand() % 24;
    }

    if (state.game.ante > 0 && state.game.ante % 8 == 0)
      state.game.blinds[2].type = rand() % (BLIND_CERULEAN_BELL - BLIND_AMBER_ACORN + 1) + BLIND_AMBER_ACORN;
    else
      state.game.blinds[2].type = rand() % (BLIND_MARK - BLIND_HOOK + 1) + BLIND_HOOK;
  } else {
    state.game.current_blind++;
  }

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

void discard_card(uint8_t index) {
  if (index >= cvector_size(state.game.hand.cards)) return;
  cvector_erase(state.game.hand.cards, index);
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
      return 4.0 * base_score;
    case BLIND_NEEDLE:
      return base_score;
    case BLIND_VIOLET_VESSEL:
      return 6.0 * base_score;

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

void buy_shop_item() {
  uint8_t item_index = state.navigation.hovered;
  NavigationSection section = get_current_section();
  uint8_t is_booster_pack = section == NAVIGATION_SHOP_BOOSTER_PACKS;
  uint8_t is_voucher = section == NAVIGATION_SHOP_VOUCHER;

  // Prevent buying items if navigation cursor is outside buyable items section
  if (!is_booster_pack && !is_voucher && section != NAVIGATION_SHOP_ITEMS) return;

  uint8_t price = is_booster_pack ? get_booster_pack_price(&state.game.shop.booster_packs[item_index])
                  : is_voucher    ? get_voucher_price(state.game.shop.vouchers[item_index])
                                  : get_shop_item_price(&state.game.shop.items[item_index]);

  if (state.game.money < price) return;

  if (is_booster_pack) {
    open_booster_pack(&state.game.shop.booster_packs[item_index]);
    cvector_erase(state.game.shop.booster_packs, item_index);
  } else if (is_voucher) {
    add_voucher_to_player(state.game.shop.vouchers[item_index]);
    if (item_index == cvector_size(state.game.shop.vouchers) - 1)
      state.game.shop.vouchers[item_index] = 0;
    else
      cvector_erase(state.game.shop.vouchers, item_index);
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
  while (cvector_size(state.game.shop.vouchers) > 1) cvector_erase(state.game.shop.vouchers, 0);

  uint8_t is_ante_first_shop = state.game.current_blind->type == BLIND_SMALL ||
                               (!state.game.blinds[0].is_active &&
                                (state.game.current_blind->type == BLIND_BIG ||
                                 (!state.game.blinds[1].is_active && state.game.current_blind->type > BLIND_BIG)));

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    Tag tag = state.game.tags[i];
    if (tag != TAG_UNCOMMON && tag != TAG_RARE) continue;

    // TODO Fix adding duplicates and wrong rarity jokers when rng utilities will be added
    ShopItem joker = (ShopItem){.type = SHOP_ITEM_JOKER, .joker = JOKERS[rand() % JOKER_COUNT]};
    cvector_push_back(state.game.shop.items, joker);

    cvector_erase(state.game.tags, i);
    i--;
  }

  fill_shop_items();

  for (uint8_t i = 0; i < 2; i++) {
    BoosterPackItem booster_pack = {.type = rand() % 5, .size = rand() % 3};
    cvector_push_back(state.game.shop.booster_packs, booster_pack);
  }

  uint8_t available_vouchers_count = 0;
  for (uint8_t i = 0; i < 32; i++) {
    if (state.game.vouchers & (1 << i)) continue;

    if (i < 16) {
      available_vouchers_count++;
      continue;
    }
    if (state.game.vouchers & (1 << (i - 16))) available_vouchers_count++;
  }
  uint32_t combined_vouchers = state.game.vouchers;

  for (int8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == TAG_VOUCHER) {
      uint8_t voucher_index = rand() % available_vouchers_count;

      for (uint8_t i = 0; i < 32; i++) {
        Voucher voucher = 1 << i;
        if (combined_vouchers & voucher) continue;

        if (voucher_index == 0) {
          cvector_insert(state.game.shop.vouchers, 0, voucher);
          combined_vouchers |= voucher;
          available_vouchers_count--;
          break;
        }
        voucher_index--;
      }

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

  if (is_ante_first_shop || state.game.round == 1) {
    uint8_t voucher_index = rand() % available_vouchers_count;

    for (uint8_t i = 0; i < 32; i++) {
      if (combined_vouchers & (1 << i)) continue;

      if (voucher_index == 0) cvector_back(state.game.shop.vouchers) = 1 << i;
      voucher_index--;
    }
  }
}

void exit_shop() {
  for (uint8_t i = 0; i < cvector_size(state.game.tags); i++) {
    if (state.game.tags[i] == TAG_COUPON) {
      cvector_erase(state.game.tags, i);
      break;
    }
  }

  change_stage(STAGE_SELECT_BLIND);
}

void select_blind() {
  state.game.round++;

  cvector_for_each(state.game.tags, Tag, tag) {
    if (*tag == TAG_JUGGLE) state.game.hand.size += 3;
  }

  shuffle_deck();
  fill_hand();
  sort_hand();

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
        // TODO Fix adding duplicates and wrong rarity jokers when rng utilities will be added
        for (uint8_t i = 0; i < 2; i++)
          add_item_to_player(&(ShopItem){.type = SHOP_ITEM_JOKER, .joker = JOKERS[rand() % JOKER_COUNT]});
        break;
      case TAG_SPEED: {
        uint8_t total_rounds = (state.game.ante - 1) * 3 + (state.game.current_blind->type == BLIND_BIG ? 1 : 2);
        if (state.game.vouchers & VOUCHER_HIEROGLYPH) total_rounds += 3;
        if (state.game.vouchers & VOUCHER_PTEROGLYPH) total_rounds += 3;
        state.game.money += 5 * (total_rounds - state.game.round);
        break;
      }
      case TAG_ORBITAL:
        state.game.poker_hands[rand() % 12].level += 3;
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
