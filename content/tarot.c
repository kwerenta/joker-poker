#include "tarot.h"

#include "../state.h"
#include "cvector.h"

const char *get_tarot_card_name(Tarot tarot) {
  switch (tarot) {
    case TAROT_FOOL:
      return "The Fool (0)";
    case TAROT_MAGICIAN:
      return "The Magician (I)";
    case TAROT_HIGH_PRIESTESS:
      return "The High Priestess (II)";
    case TAROT_EMPRESS:
      return "The Empress (III)";
    case TAROT_EMPEROR:
      return "The Emperor (IV)";
    case TAROT_HIEROPHANT:
      return "The Hierophant (V)";
    case TAROT_LOVERS:
      return "The Lovers (VI)";
    case TAROT_CHARIOT:
      return "The Chariot (VII)";
    case TAROT_JUSTICE:
      return "Justice (VIII)";
    case TAROT_HERMIT:
      return "The Hermit (IX)";
    case TAROT_WHEEL_OF_FORTUNE:
      return "The Wheel of Fortune (X)";
    case TAROT_STRENGTH:
      return "Strength (XI)";
    case TAROT_HANGED_MAN:
      return "The Hanged Man (XII)";
    case TAROT_DEATH:
      return "Death (XIII)";
    case TAROT_TEMPERANCE:
      return "Temperance (XIV)";
    case TAROT_DEVIL:
      return "The Devil (XV)";
    case TAROT_TOWER:
      return "The Tower (XVI)";
    case TAROT_STAR:
      return "The Star (XVII)";
    case TAROT_MOON:
      return "The Moon (XVIII)";
    case TAROT_SUN:
      return "The Sun (XIX)";
    case TAROT_JUDGEMENT:
      return "Judgement (XX)";
    case TAROT_WORLD:
      return "The World (XXI)";
  }
}

const char *get_tarot_card_description(Tarot tarot) {
  switch (tarot) {
    case TAROT_FOOL:
      return "Creates the last Tarot or Planet card used during this run (The Fool excluded)";
    case TAROT_MAGICIAN:
      return "Enhances 2 selected cards to Lucky Cards";
    case TAROT_HIGH_PRIESTESS:
      return "Creates up to 2 random Planet cards (Must have room)";
    case TAROT_EMPRESS:
      return "Enhances 2 selected cards to Mult Cards";
    case TAROT_EMPEROR:
      return "Creates up to 2 random Tarot cards (Must have room)";
    case TAROT_HIEROPHANT:
      return "Enhances 2 selected cards to Bonus Cards";
    case TAROT_LOVERS:
      return "Enhances 1 selected card into a Wild Card";
    case TAROT_CHARIOT:
      return "Enhances 1 selected card into a Steel Card";
    case TAROT_JUSTICE:
      return "Enhances 1 selected card into a Glass Card";
    case TAROT_HERMIT:
      return "Doubles money (Max of $20)";
    case TAROT_WHEEL_OF_FORTUNE:
      return "1 in 4 chance to add Foil, Holographic, or Polychrome edition to a random Joker";
    case TAROT_STRENGTH:
      return "Increases rank of up to 2 selected cards by 1";
    case TAROT_HANGED_MAN:
      return "Destroys up to 2 selected cards";
    case TAROT_DEATH:
      return "Select 2 cards, convert the left card into the right card (Drag to rearrange)";
    case TAROT_TEMPERANCE:
      return "Gives the total sell value of all current Jokers (Max of $50)";
    case TAROT_DEVIL:
      return "Enhances 1 selected card into a Gold Card";
    case TAROT_TOWER:
      return "Enhances 1 selected card into a Stone Card";
    case TAROT_STAR:
      return "Converts up to 3 selected cards to Diamonds";
    case TAROT_MOON:
      return "Converts up to 3 selected cards to Clubs";
    case TAROT_SUN:
      return "Converts up to 3 selected cards to Hearts";
    case TAROT_JUDGEMENT:
      return "Creates a random Joker card (Must have room)";
    case TAROT_WORLD:
      return "Converts up to 3 selected cards to Spades";
  }
}

uint8_t get_tarot_max_selected(Tarot tarot) {
  switch (tarot) {
    case TAROT_FOOL:
    case TAROT_EMPEROR:
    case TAROT_HERMIT:
    case TAROT_WHEEL_OF_FORTUNE:
    case TAROT_TEMPERANCE:
    case TAROT_JUDGEMENT:
      return 0;

    case TAROT_LOVERS:
    case TAROT_CHARIOT:
    case TAROT_JUSTICE:
    case TAROT_DEVIL:
    case TAROT_TOWER:
      return 1;

    case TAROT_MAGICIAN:
    case TAROT_HIGH_PRIESTESS:
    case TAROT_EMPRESS:
    case TAROT_HIEROPHANT:
    case TAROT_STRENGTH:
    case TAROT_HANGED_MAN:
    case TAROT_DEATH:
      return 2;

    case TAROT_STAR:
    case TAROT_MOON:
    case TAROT_SUN:
    case TAROT_WORLD:
      return 3;
  }
}

void tarot_change_suit(Card **selected_cards, uint8_t selected_count, Suit new_suit) {
  for (uint8_t i = 0; i < selected_count; i++) {
    cvector_for_each(state.game.full_deck, Card, card) {
      if (compare_cards(selected_cards[i], card)) {
        selected_cards[i]->suit = new_suit;
        break;
      }
    }

    selected_cards[i]->suit = new_suit;
  }
}

void use_tarot_card(Tarot tarot) {
  Card *selected_cards[3] = {0};
  uint8_t selected_count = 0;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected == 0) continue;

    selected_cards[selected_count] = card;
    selected_count++;
    if (selected_count > 3) return;
  }

  uint8_t max_selected_count = get_tarot_max_selected(tarot);
  if ((max_selected_count != 0 && selected_count > max_selected_count) ||
      (tarot == TAROT_DEATH && selected_count != max_selected_count))
    return;

  switch (tarot) {
    case TAROT_STAR:
      tarot_change_suit(selected_cards, selected_count, SUIT_DIAMONDS);
      break;
    case TAROT_MOON:
      tarot_change_suit(selected_cards, selected_count, SUIT_CLUBS);
      break;
    case TAROT_SUN:
      tarot_change_suit(selected_cards, selected_count, SUIT_HEARTS);
      break;
    case TAROT_WORLD:
      tarot_change_suit(selected_cards, selected_count, SUIT_SPADES);
      break;

    default:
      // TODO Implement rest of the tarot cards
      return;
  }
}
