#include "spectral.h"

#include "../random.h"
#include "../state.h"
#include "cvector.h"

const char *get_spectral_card_name(Spectral spectral) {
  switch (spectral) {
    case SPECTRAL_FAMILIAR:
      return "Familiar";
    case SPECTRAL_GRIM:
      return "Grim";
    case SPECTRAL_INCANTATION:
      return "Incantation";
    case SPECTRAL_TALISMAN:
      return "Talisman";
    case SPECTRAL_AURA:
      return "Aura";
    case SPECTRAL_WRAITH:
      return "Wraith NOT IMPLEMENTED";
    case SPECTRAL_SIGIL:
      return "Sigil";
    case SPECTRAL_OUIJA:
      return "Ouija";
    case SPECTRAL_ECTOPLASM:
      return "Ectoplasm NOT IMPLEMENTED";
    case SPECTRAL_IMMOLATE:
      return "Immolate";
    case SPECTRAL_ANKH:
      return "Ankh";
    case SPECTRAL_DEJA_VU:
      return "Deja Vu";
    case SPECTRAL_HEX:
      return "Hex";
    case SPECTRAL_TRANCE:
      return "Trance";
    case SPECTRAL_MEDIUM:
      return "Medium";
    case SPECTRAL_CRYPTID:
      return "Cryptid";
    case SPECTRAL_SOUL:
      return "The Soul NOT IMPLEMENTED";
    case SPECTRAL_BLACK_HOLE:
      return "Black Hole";
  }
};

const char *get_spectral_card_description(Spectral spectral) {
  switch (spectral) {
    case SPECTRAL_FAMILIAR:
      return "Destroy 1 random card in your hand, but add 3 random Enhanced face cards instead.";
    case SPECTRAL_GRIM:
      return "Destroy 1 random card in your hand, but add 2 random Enhanced Aces instead.";
    case SPECTRAL_INCANTATION:
      return "Destroy 1 random card in your hand, but add 4 random Enhanced numbered cards instead.";
    case SPECTRAL_TALISMAN:
      return "Add a Gold Seal to 1 selected card.";
    case SPECTRAL_AURA:
      return "Add Foil, Holographic, or Polychrome edition (determined at random) to 1 selected card in hand.";
    case SPECTRAL_WRAITH:
      return "Creates a random Rare Joker (must have room), but sets money to $0.";
    case SPECTRAL_SIGIL:
      return "Converts all cards in hand to a single random suit.";
    case SPECTRAL_OUIJA:
      return "Converts all cards in hand to a single random rank, but -1 Hand Size.";
    case SPECTRAL_ECTOPLASM:
      return "Add Negative to a random Joker, but -1 Hand Size, plus another -1 hand size for each time Ectoplasm has "
             "been used this run";
    case SPECTRAL_IMMOLATE:
      return "Destroys 5 random cards in hand, but gain $20.";
    case SPECTRAL_ANKH:
      return "Creates a copy of 1 of your Jokers at random, then destroys the others, leaving you with two identical "
             "Jokers.";
    case SPECTRAL_DEJA_VU:
      return "Adds a Red Seal to 1 selected card.";
    case SPECTRAL_HEX:
      return "Adds Polychrome to a random Joker, and destroys the rest.";
    case SPECTRAL_TRANCE:
      return "Adds a Blue Seal to 1 selected card.";
    case SPECTRAL_MEDIUM:
      return "Adds a Purple Seal to 1 selected card.";
    case SPECTRAL_CRYPTID:
      return "Creates 2 exact copies (including Enhancements, Editions and Seals) of a selected card in your hand.";
    case SPECTRAL_SOUL:
      return "Creates a Legendary Joker (Must have room)";
    case SPECTRAL_BLACK_HOLE:
      return "Upgrades every poker hand (including secret hands not yet discovered) by one level.";
  }
};

uint8_t get_spectral_max_selected(Spectral spectral) {
  switch (spectral) {
    case SPECTRAL_TALISMAN:
    case SPECTRAL_AURA:
    case SPECTRAL_DEJA_VU:
    case SPECTRAL_TRANCE:
    case SPECTRAL_MEDIUM:
    case SPECTRAL_CRYPTID:
      return 1;

    default:
      return 0;
  }
}

void destroy_random_card() {
  if (cvector_size(state.game.hand.cards) == 0) return;

  uint8_t destroy_index = random_vector_index(state.game.hand.cards);

  for (uint8_t i = 0; i < cvector_size(state.game.full_deck); i++) {
    if (compare_cards(&state.game.full_deck[i], &state.game.hand.cards[destroy_index])) {
      cvector_erase(state.game.full_deck, i);
      break;
    }
  }

  cvector_erase(state.game.hand.cards, destroy_index);
}
void add_card_to_deck(Suit suit, Rank rank, Edition edition, Enhancement enhancement, Seal seal) {
  Card card = create_card(suit, rank, edition, enhancement, seal);
  cvector_push_back(state.game.hand.cards, card);
  cvector_push_back(state.game.full_deck, card);
}

uint8_t use_spectral_card(Spectral spectral) {
  Card *selected_cards[2] = {0};
  uint8_t selected_count = 0;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected == 0) continue;

    selected_cards[selected_count] = card;
    selected_count++;
    if (selected_count > 2) return 0;
  }

  uint8_t max_selected_count = get_spectral_max_selected(spectral);
  if (selected_count != max_selected_count) return 0;

  switch (spectral) {
    case SPECTRAL_FAMILIAR:
      if (state.game.hand.size == 1) return 0;
      destroy_random_card();
      for (uint8_t i = 0; i < 3; i++)
        add_card_to_deck(random_max_value(3), random_in_range(10, 12), EDITION_BASE, random_in_range(1, 8), SEAL_NONE);
      break;

    case SPECTRAL_GRIM:
      if (state.game.hand.size == 1) return 0;
      destroy_random_card();
      for (uint8_t i = 0; i < 2; i++)
        add_card_to_deck(random_max_value(3), RANK_ACE, EDITION_BASE, random_in_range(1, 8), SEAL_NONE);
      break;

    case SPECTRAL_INCANTATION:
      if (state.game.hand.size == 1) return 0;
      destroy_random_card();
      for (uint8_t i = 0; i < 4; i++)
        add_card_to_deck(random_max_value(3), random_in_range(1, 9), EDITION_BASE, random_in_range(1, 8), SEAL_NONE);
      break;

    case SPECTRAL_TALISMAN:
      selected_cards[0]->seal = SEAL_GOLD;
      break;

    case SPECTRAL_AURA:
      selected_cards[0]->edition = random_in_range(1, 3);
      break;

    case SPECTRAL_WRAITH:
      // TODO Add this when some rare jokers will be added
      break;

    case SPECTRAL_SIGIL: {
      if (state.game.hand.size == 1) return 0;
      Suit new_suit = random_max_value(3);

      cvector_for_each(state.game.hand.cards, Card, hand_card) {
        cvector_for_each(state.game.full_deck, Card, card) {
          if (compare_cards(hand_card, card)) {
            card->suit = new_suit;
            card->was_played = 0;
            hand_card->suit = new_suit;
            hand_card->was_played = 0;
            break;
          }
        }
      }
      break;
    }

    case SPECTRAL_OUIJA: {
      if (state.game.hand.size == 1) return 0;
      Rank new_rank = random_max_value(12);
      state.game.hand.size--;

      cvector_for_each(state.game.hand.cards, Card, hand_card) {
        cvector_for_each(state.game.full_deck, Card, card) {
          if (compare_cards(hand_card, card)) {
            card->rank = new_rank;
            card->was_played = 0;
            hand_card->rank = new_rank;
            hand_card->was_played = 0;
            break;
          }
        }
      }
      break;
    }

    case SPECTRAL_ECTOPLASM:
      // TODO Add when negative jokers will be implemented
      break;

    case SPECTRAL_IMMOLATE:
      if (state.game.hand.size == 1) return 0;
      state.game.money += 20;
      for (uint8_t i = 0; i < 5; i++) destroy_random_card();
      break;

    case SPECTRAL_ANKH: {
      uint8_t joker_to_copy = random_vector_index(state.game.jokers.cards);
      Joker joker = state.game.jokers.cards[joker_to_copy];
      // TODO Don't remove eternal jokers when they will be added
      cvector_clear(state.game.jokers.cards);
      for (uint8_t i = 0; i < 2; i++) cvector_push_back(state.game.jokers.cards, joker);
      break;
    }

    case SPECTRAL_DEJA_VU:
      selected_cards[0]->seal = SEAL_RED;
      break;

    case SPECTRAL_HEX: {
      // TODO Ignore jokers with editions
      uint8_t joker_to_upgrade = random_vector_index(state.game.jokers.cards);
      Joker joker = state.game.jokers.cards[joker_to_upgrade];
      // TODO Don't remove eternal jokers when they will be added
      cvector_clear(state.game.jokers.cards);

      joker.edition = EDITION_POLYCHROME;
      cvector_push_back(state.game.jokers.cards, joker);
      break;
    }

    case SPECTRAL_TRANCE:
      selected_cards[0]->seal = SEAL_BLUE;
      break;

    case SPECTRAL_MEDIUM:
      selected_cards[0]->seal = SEAL_PURPLE;
      break;

    case SPECTRAL_CRYPTID: {
      Card *card = selected_cards[0];
      for (uint8_t i = 0; i < 2; i++)
        add_card_to_deck(card->suit, card->rank, card->edition, card->enhancement, card->seal);
      break;
    }

    case SPECTRAL_SOUL:
      // TODO Add this when legendary jokers will be added
      break;

    case SPECTRAL_BLACK_HOLE:
      for (uint8_t i = 0; i < 12; i++) state.game.poker_hands[i].level++;
      break;
  }

  if (max_selected_count != 0) deselect_all_cards();
  if (state.stage == STAGE_GAME && state.game.current_blind->type > BLIND_BIG) {
    disable_boss_blind();
    enable_boss_blind();
  }
  return 1;
}
