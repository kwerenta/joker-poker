#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "lib/cvector.h"
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

  state.game.shop.selected_card = 0;

  state.stage = STAGE_GAME;
}

void game_destroy() {
  cvector_free(state.game.deck);
  state.game.deck = NULL;

  cvector_free(state.game.full_deck);
  state.game.full_deck = NULL;

  cvector_free(state.game.hand.cards);
  state.game.hand.cards = NULL;

  cvector_free(state.game.jokers.cards);
  state.game.jokers.cards = NULL;

  cvector_free(state.game.shop.items);
  state.game.shop.items = NULL;

  cvector_free(state.game.booster_pack.content);
  state.game.booster_pack.content = NULL;
}

Card create_card(Suit suit, Rank rank, Edition edition, Enhancement enhancement) {
  uint16_t chips = rank == RANK_ACE ? 11 : rank + 1;
  if (rank != RANK_ACE && chips > 10) {
    chips = 10;
  }

  return (Card){
      .suit = suit, .rank = rank, .chips = chips, .edition = edition, .enhancement = enhancement, .selected = 0};
}

void draw_card() {
  cvector_push_back(state.game.hand.cards, cvector_back(state.game.deck));
  cvector_pop_back(state.game.deck);
}

void fill_hand() {
  while (cvector_size(state.game.hand.cards) < state.game.hand.size) {
    draw_card();
  }
}

void play_hand() {
  if (state.game.hands == 0 || state.game.selected_hand.count == 0)
    return;

  update_scoring_hand();

  for (uint8_t i = 0; i < 5; i++) {
    Card *card = state.game.selected_hand.scoring_cards[i];
    if (card == NULL)
      continue;

    if (card->enhancement != ENHANCEMENT_STONE)
      state.game.selected_hand.scoring.chips += card->chips;

    update_scoring_edition(card->edition);

    switch (card->enhancement) {
    case ENHANCEMENT_NONE:
    case ENHANCEMENT_GOLD:
    case ENHANCEMENT_WILD:
    case ENHANCEMENT_STEEL:
      break;

    case ENHANCEMENT_BONUS:
      state.game.selected_hand.scoring.chips += 30;
      break;

    case ENHANCEMENT_MULT:
      state.game.selected_hand.scoring.mult += 4;
      break;

    case ENHANCEMENT_GLASS:
      state.game.selected_hand.scoring.mult *= 2;

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
      state.game.selected_hand.scoring.chips += 50;
      break;

    case ENHANCEMENT_LUCKY:
      if (rand() % 5 == 0)
        state.game.selected_hand.scoring.mult += 20;

      if (rand() % 15 == 0)
        state.game.money += 20;
      break;
    }
  }

  cvector_for_each(state.game.jokers.cards, Joker, joker) {
    if (joker->activation_type == ACTIVATION_INDEPENDENT)
      joker->activate();

    update_scoring_edition(joker->edition);
  }

  remove_selected_cards();
  state.game.hands--;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->enhancement == ENHANCEMENT_STEEL)
      state.game.selected_hand.scoring.mult *= 1.5;
  }

  state.game.score += state.game.selected_hand.scoring.chips * state.game.selected_hand.scoring.mult;

  double required_score = get_required_score(state.game.ante, state.game.blind);

  if (state.game.score >= required_score) {
    // interest
    state.game.money += (state.game.money) / 5;

    cvector_for_each(state.game.hand.cards, Card, card) {
      if (card->enhancement == ENHANCEMENT_GOLD)
        state.game.money += 3;
    }

    state.stage = STAGE_SHOP;
    restock_shop();

    state.game.money += 1 * state.game.hands + get_blind_money(state.game.blind);
  } else if (state.game.hands == 0) {
    state.stage = STAGE_GAME_OVER;
  } else {
    fill_hand();
  }
}

void update_scoring_edition(Edition edition) {
  switch (edition) {
  case EDITION_BASE:
    break;
  case EDITION_FOIL:
    state.game.selected_hand.scoring.chips += 50;
    break;
  case EDITION_HOLOGRAPHIC:
    state.game.selected_hand.scoring.mult += 10;
    break;
  case EDITION_POLYCHROME:
    state.game.selected_hand.scoring.mult *= 1.5;
    break;
  case EDITION_NEGATIVE:
    // TODO implement negative jokers and cards when consumable slots will be added
    break;
  }
}

void discard_hand() {
  if (state.game.selected_hand.count == 0 || state.game.discards == 0)
    return;

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

void set_hovered_card(uint8_t *hovered, uint8_t new_position) {
  if (new_position >= cvector_size(state.game.hand.cards))
    return;

  *hovered = new_position;
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
    if (card->selected == 0)
      continue;

    (*selected_count)++;

    if (*selected_count == 5)
      return;
  }

  (*selected_count)++;

  hand->cards[index].selected = 1;

  update_scoring_hand();
}

void deselect_all_cards() {
  cvector_for_each(state.game.hand.cards, Card, card) { card->selected = 0; }
  state.game.selected_hand.count = 0;
}

void move_card_in_hand(uint8_t *hovered, uint8_t new_position) {
  Hand *hand = &state.game.hand;

  if (new_position >= cvector_size(hand->cards))
    return;

  Card temp = hand->cards[*hovered];
  hand->cards[*hovered] = hand->cards[new_position];
  hand->cards[new_position] = temp;

  *hovered = new_position;
}

int compare_by_rank(const void *a, const void *b) {
  Rank a_rank = ((const Card *)a)->rank, b_rank = ((const Card *)b)->rank;
  if (a_rank == RANK_ACE) {
    a_rank = RANK_KING + 1;
  }
  if (b_rank == RANK_ACE) {
    b_rank = RANK_KING + 1;
  }

  int by_rank = b_rank - a_rank;
  if (by_rank == 0) {
    return ((const Card *)a)->suit - ((const Card *)b)->suit;
  }

  return by_rank;
}
int compare_by_suit(const void *a, const void *b) {
  int by_suit = ((const Card *)a)->suit - ((const Card *)b)->suit;
  if (by_suit == 0) {
    return compare_by_rank(a, b);
  }

  return by_suit;
}

void sort_hand(uint8_t by_suit) {
  int (*comparator)(const void *a, const void *b) = compare_by_rank;

  if (by_suit == 1) {
    comparator = compare_by_suit;
  }

  qsort(state.game.hand.cards, cvector_size(state.game.hand.cards), sizeof(Card), comparator);
}

PokerHand evaluate_hand() {
  const Hand *hand = &state.game.hand;

  uint8_t rank_counts[13] = {};
  uint8_t suit_counts[4] = {};
  const uint8_t selected_count = state.game.selected_hand.count;

  // 2 of kind, 3 of kind, 4 of kind, 5 of kind
  uint8_t x_of_kind[4] = {};
  uint8_t has_flush = 0;
  uint8_t has_straight = 0;
  uint8_t has_ace = 0;

  Rank highest_card = RANK_TWO, lowest_card = RANK_KING;

  cvector_for_each(hand->cards, Card, card) {
    if (card->selected == 0 || card->enhancement == ENHANCEMENT_STONE)
      continue;

    if (card->rank == RANK_ACE)
      has_ace = 1;

    if (card->rank != RANK_ACE && card->rank > highest_card)
      highest_card = card->rank;

    if (card->rank != RANK_ACE && card->rank < lowest_card)
      lowest_card = card->rank;

    rank_counts[card->rank]++;

    if (card->enhancement == ENHANCEMENT_WILD)
      for (uint8_t i = 0; i < 4; i++)
        suit_counts[i]++;
    else
      suit_counts[card->suit]++;

    if (rank_counts[card->rank] > 1) {
      // this means there are duplicates in selected ranks,
      // so straight is not possible
      has_straight = 2;
      x_of_kind[rank_counts[card->rank] - 2]++;
      if (rank_counts[card->rank] > 2)
        x_of_kind[rank_counts[card->rank] - 3]--;
    }
  }

  for (uint8_t i = 0; i < 4; i++)
    if (suit_counts[i] == 5)
      has_flush = 1;

  // A 2 3 4 5 is also valid straight, so it needs to be checked
  if (has_straight == 0 && selected_count == 5 &&
      ((has_ace == 0 && highest_card - lowest_card == 4) ||
       (has_ace == 1 && (13 - lowest_card == 4 || highest_card == 4))))
    has_straight = 1;

  if (has_flush == 1 && x_of_kind[5 - 2] > 0)
    return HAND_FLUSH_FIVE;

  if (has_flush == 1 && x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1)
    return HAND_FLUSH_HOUSE;

  if (x_of_kind[5 - 2] == 1)
    return HAND_FIVE_OF_KIND;

  if (has_flush == 1 && has_straight == 1)
    return HAND_STRAIGHT_FLUSH;

  if (x_of_kind[4 - 2] == 1)
    return HAND_FOUR_OF_KIND;

  if (x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1)
    return HAND_FULL_HOUSE;

  if (has_flush == 1)
    return HAND_FLUSH;

  if (has_straight == 1)
    return HAND_STRAIGHT;

  if (x_of_kind[3 - 2] == 1)
    return HAND_THREE_OF_KIND;

  if (x_of_kind[2 - 2] >= 2)
    return HAND_TWO_PAIR;

  if (x_of_kind[2 - 2] >= 1)
    return HAND_PAIR;

  return HAND_HIGH_CARD;
}

void update_scoring_hand() {
  PokerHand poker_hand = evaluate_hand();

  state.game.selected_hand.poker_hand = poker_hand;
  const Hand *hand = &state.game.hand;
  Card *selected_cards[5] = {};
  Card **scoring_cards = state.game.selected_hand.scoring_cards;

  // Clear previous scoring cards
  memset(scoring_cards, 0, 5 * sizeof(Card *));

  uint8_t j = 0;
  cvector_for_each(hand->cards, Card, card) {
    if (card->selected == 0)
      continue;

    selected_cards[j] = card;
    j++;
  }

  if (selected_cards[0] == NULL)
    return;

  // All of those hands require 5 selected cards
  if (poker_hand == HAND_FLUSH_FIVE || poker_hand == HAND_FLUSH_HOUSE || poker_hand == HAND_FIVE_OF_KIND ||
      poker_hand == HAND_STRAIGHT_FLUSH || poker_hand == HAND_FULL_HOUSE || poker_hand == HAND_FLUSH ||
      poker_hand == HAND_STRAIGHT) {
    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL)
        return;

      scoring_cards[i] = selected_cards[i];
    }
  }

  uint8_t rank_counts[13] = {};
  uint8_t highest_card_index = 0xFF;
  Rank scoring_rank = selected_cards[0]->rank;
  uint8_t scoring_count = 0;
  for (uint8_t i = 0; i < 5; i++) {
    if (selected_cards[i] == NULL)
      break;

    if (selected_cards[i]->enhancement == ENHANCEMENT_STONE)
      continue;

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

  if (poker_hand == HAND_FOUR_OF_KIND || poker_hand == HAND_THREE_OF_KIND || poker_hand == HAND_PAIR ||
      poker_hand == HAND_TWO_PAIR) {
    uint8_t j = 0;

    Rank second_scoring_rank = scoring_rank;
    if (poker_hand == HAND_TWO_PAIR) {
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
  if (poker_hand == HAND_HIGH_CARD) {
    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL || (i != highest_card_index && selected_cards[i]->enhancement != ENHANCEMENT_STONE))
        continue;

      scoring_cards[j] = selected_cards[i];
      j++;
    }
  }

  state.game.selected_hand.scoring = get_poker_hand_total_scoring(poker_hand);
}

PokerHandScoring get_poker_hand_base_scoring(PokerHand hand) {
  switch (hand) {
  case HAND_FLUSH_FIVE:
    return (PokerHandScoring){.mult = 16, .chips = 160};
  case HAND_FLUSH_HOUSE:
    return (PokerHandScoring){.mult = 14, .chips = 140};
  case HAND_FIVE_OF_KIND:
    return (PokerHandScoring){.mult = 12, .chips = 120};
  case HAND_STRAIGHT_FLUSH:
    return (PokerHandScoring){.mult = 8, .chips = 100};
  case HAND_FOUR_OF_KIND:
    return (PokerHandScoring){.mult = 7, .chips = 60};
  case HAND_FULL_HOUSE:
    return (PokerHandScoring){.mult = 4, .chips = 40};
  case HAND_FLUSH:
    return (PokerHandScoring){.mult = 4, .chips = 35};
  case HAND_STRAIGHT:
    return (PokerHandScoring){.mult = 4, .chips = 30};
  case HAND_THREE_OF_KIND:
    return (PokerHandScoring){.mult = 3, .chips = 30};
  case HAND_TWO_PAIR:
    return (PokerHandScoring){.mult = 2, .chips = 20};
  case HAND_PAIR:
    return (PokerHandScoring){.mult = 2, .chips = 10};
  case HAND_HIGH_CARD:
    return (PokerHandScoring){.mult = 1, .chips = 5};
  }
}

PokerHandScoring get_planet_card_base_scoring(PokerHand hand) {
  switch (hand) {
  case HAND_FLUSH_FIVE:
    return (PokerHandScoring){.mult = 3, .chips = 50};
  case HAND_FLUSH_HOUSE:
    return (PokerHandScoring){.mult = 4, .chips = 40};
  case HAND_FIVE_OF_KIND:
    return (PokerHandScoring){.mult = 3, .chips = 35};
  case HAND_STRAIGHT_FLUSH:
    return (PokerHandScoring){.mult = 4, .chips = 40};
  case HAND_FOUR_OF_KIND:
    return (PokerHandScoring){.mult = 3, .chips = 30};
  case HAND_FULL_HOUSE:
    return (PokerHandScoring){.mult = 2, .chips = 25};
  case HAND_FLUSH:
    return (PokerHandScoring){.mult = 2, .chips = 15};
  case HAND_STRAIGHT:
    return (PokerHandScoring){.mult = 3, .chips = 30};
  case HAND_THREE_OF_KIND:
    return (PokerHandScoring){.mult = 2, .chips = 20};
  case HAND_TWO_PAIR:
    return (PokerHandScoring){.mult = 1, .chips = 20};
  case HAND_PAIR:
    return (PokerHandScoring){.mult = 1, .chips = 15};
  case HAND_HIGH_CARD:
    return (PokerHandScoring){.mult = 1, .chips = 10};
  }
}

PokerHandScoring get_poker_hand_total_scoring(PokerHand poker_hand) {
  PokerHandScoring scoring = get_poker_hand_base_scoring(poker_hand);
  PokerHandScoring planet_scoring = get_planet_card_base_scoring(poker_hand);

  scoring.chips += planet_scoring.chips * state.game.poker_hands[poker_hand];
  scoring.mult += planet_scoring.mult * state.game.poker_hands[poker_hand];

  return scoring;
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

uint8_t add_item_to_player(ShopItem *item) {
  switch (item->type) {
  case SHOP_ITEM_JOKER:
    if (cvector_size(state.game.jokers.cards) >= state.game.jokers.size)
      return 0;

    cvector_push_back(state.game.jokers.cards, item->joker);
    break;

  case SHOP_ITEM_CARD:
    cvector_push_back(state.game.full_deck, item->card);
    break;

  case SHOP_ITEM_PLANET:
    state.game.poker_hands[item->planet] += 1;
    break;

  case SHOP_ITEM_BOOSTER_PACK:
    open_booster_pack(item->booster_pack);
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
  case SHOP_ITEM_BOOSTER_PACK:
    return 4 + item->booster_pack.size * 2;
  case SHOP_ITEM_JOKER:
    return item->joker.base_price;
  }
}

void buy_shop_item() {
  uint8_t shopCount = cvector_size(state.game.shop.items);
  ShopItem *item = &state.game.shop.items[state.game.shop.selected_card];
  uint8_t price = get_shop_item_price(item);

  if (state.game.shop.selected_card >= shopCount || state.game.money < price)
    return;

  if (add_item_to_player(item) == 0)
    return;

  state.game.money -= price;
  cvector_erase(state.game.shop.items, state.game.shop.selected_card);
  shopCount--;

  if (state.game.shop.selected_card >= shopCount)
    state.game.shop.selected_card = shopCount - 1;
}

void open_booster_pack(BoosterPackItem booster_pack) {
  cvector_clear(state.game.booster_pack.content);
  state.game.booster_pack.item = booster_pack;
  state.game.booster_pack.hovered_item = 0;
  state.stage = STAGE_BOOSTER_PACK;

  uint8_t count = booster_pack.size == BOOSTER_PACK_NORMAL ? 3 : 5;
  for (uint8_t i = 0; i < count; i++) {
    BoosterPackContent content = {.selected = 0};

    switch (booster_pack.type) {
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

void submit_booster_pack() {
  ShopItem item = {};

  cvector_for_each(state.game.booster_pack.content, BoosterPackContent, content) {
    if (content->selected == 0)
      continue;

    switch (state.game.booster_pack.item.type) {
    case BOOSTER_PACK_STANDARD:
      item.type = SHOP_ITEM_CARD;
      item.card = content->card;
      break;

    case BOOSTER_PACK_BUFFON:
      item.type = SHOP_ITEM_JOKER;
      item.joker = content->joker;
      break;

    case BOOSTER_PACK_CELESTIAL:
      item.type = SHOP_ITEM_PLANET;
      item.planet = content->planet;
      break;
    }

    add_item_to_player(&item);
  }

  state.stage = STAGE_SHOP;
}

void toggle_booster_pack_item_select() {
  BoosterPackContent *content = &state.game.booster_pack.content[state.game.booster_pack.hovered_item];

  if (content->selected == 1) {
    content->selected = 0;
    return;
  }

  uint8_t selected_count = 0;
  const uint8_t max_count = state.game.booster_pack.item.size == BOOSTER_PACK_MEGA ? 2 : 1;

  cvector_for_each(state.game.booster_pack.content, BoosterPackContent, c) {
    if (c->selected == 1)
      selected_count++;
  }

  if (selected_count < max_count)
    content->selected = 1;
}

void restock_shop() {
  cvector_clear(state.game.shop.items);
  state.game.shop.selected_card = 0;

  ShopItem card = {.type = SHOP_ITEM_CARD,
                   .card = create_card(rand() % 4, rand() % 13, EDITION_BASE, ENHANCEMENT_NONE)};
  cvector_push_back(state.game.shop.items, card);

  ShopItem joker = {.type = SHOP_ITEM_JOKER, .joker = JOKERS[rand() % JOKER_COUNT]};
  cvector_push_back(state.game.shop.items, joker);

  ShopItem planet = {.type = SHOP_ITEM_PLANET, .planet = rand() % 12};
  cvector_push_back(state.game.shop.items, planet);

  ShopItem pack = {.type = SHOP_ITEM_BOOSTER_PACK, .booster_pack = {.type = rand() % 3, .size = rand() % 3}};
  cvector_push_back(state.game.shop.items, pack);
}

void exit_shop() {
  state.game.round++;
  state.game.blind++;

  if (state.game.blind > 2) {
    state.game.blind = 0;
    state.game.ante++;
  }

  state.game.score = 0;

  // Reset hand and deck for new blind
  state.game.hands = 4;
  state.game.discards = 2;
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck, state.game.deck);
  shuffle_deck();

  fill_hand();

  state.stage = STAGE_GAME;
}
