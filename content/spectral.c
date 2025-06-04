#include "spectral.h"

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
      return "Wraith";
    case SPECTRAL_SIGIL:
      return "Sigil";
    case SPECTRAL_OUIJA:
      return "Ouija";
    case SPECTRAL_ECTOPLASM:
      return "Ectoplasm";
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
      return "The Soul";
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
             "been used this run.";
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

  if (max_selected_count != 0) deselect_all_cards();
  return 1;
}
