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
      return "The Wheel of Fortune (X) NOT IMPLEMENTED";
    case TAROT_STRENGTH:
      return "Strength (XI)";
    case TAROT_HANGED_MAN:
      return "The Hanged Man (XII)";
    case TAROT_DEATH:
      return "Death (XIII)";
    case TAROT_TEMPERANCE:
      return "Temperance (XIV) NOT IMPLEMENTED";
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
      return "Select 2 cards, convert the left card into the right card (Move to rearrange)";
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
    case TAROT_HIGH_PRIESTESS:
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

void tarot_create_consumable(ConsumableType type) {
  uint8_t available_space = state.game.consumables.size - cvector_size(state.game.consumables.items);
  for (uint8_t i = 0; i < available_space; i++) {
    Consumable consumable = {.type = type};
    if (type == CONSUMABLE_PLANET)
      consumable.planet = rand() % 12;
    else if (type == CONSUMABLE_TAROT)
      consumable.tarot = rand() % 22;

    cvector_push_back(state.game.consumables.items, consumable);
  }
}

void tarot_change_enhancement(Card **selected_cards, uint8_t selected_count, Enhancement new_enhancement) {
  for (uint8_t i = 0; i < selected_count; i++) {
    cvector_for_each(state.game.full_deck, Card, card) {
      if (compare_cards(selected_cards[i], card)) {
        card->enhancement = new_enhancement;
        selected_cards[i]->enhancement = new_enhancement;
        break;
      }
    }
  }
}

void tarot_change_suit(Card **selected_cards, uint8_t selected_count, Suit new_suit) {
  for (uint8_t i = 0; i < selected_count; i++) {
    cvector_for_each(state.game.full_deck, Card, card) {
      if (compare_cards(selected_cards[i], card)) {
        card->suit = new_suit;
        selected_cards[i]->suit = new_suit;
        break;
      }
    }
  }
}

uint8_t use_tarot_card(Tarot tarot) {
  Card *selected_cards[3] = {0};
  uint8_t selected_count = 0;

  cvector_for_each(state.game.hand.cards, Card, card) {
    if (card->selected == 0) continue;

    selected_cards[selected_count] = card;
    selected_count++;
    if (selected_count > 3) return 0;
  }

  uint8_t max_selected_count = get_tarot_max_selected(tarot);
  if ((selected_count == 0 && max_selected_count != 0) ||
      (max_selected_count != 0 && selected_count > max_selected_count) ||
      (tarot == TAROT_DEATH && selected_count != max_selected_count))
    return 0;

  switch (tarot) {
    case TAROT_FOOL:
      if (cvector_size(state.game.consumables.items) >= state.game.consumables.size ||
          state.game.fool_last_used.was_used == 0 ||
          (state.game.fool_last_used.consumable.type == CONSUMABLE_TAROT &&
           state.game.fool_last_used.consumable.tarot == TAROT_FOOL))
        return 0;
      cvector_push_back(state.game.consumables.items, state.game.fool_last_used.consumable);
      break;
    case TAROT_HERMIT:
      if (state.game.money > 0) state.game.money += state.game.money > 20 ? 20 : state.game.money;
      break;
    case TAROT_WHEEL_OF_FORTUNE:
      // TODO Add this when RNG utilities will be added
      break;
    case TAROT_STRENGTH:
      for (uint8_t i = 0; i < selected_count; i++) {
        cvector_for_each(state.game.full_deck, Card, card) {
          if (compare_cards(selected_cards[i], card)) {
            card->rank = (card->rank + 1) % 12;
            selected_cards[i]->rank = card->rank;
            break;
          }
        }
      }
      break;
    case TAROT_HANGED_MAN:
      for (uint8_t i = 0; i < selected_count; i++) {
        for (size_t j = 0; j < cvector_size(state.game.full_deck); j++) {
          if (compare_cards(selected_cards[i], &state.game.full_deck[j])) {
            cvector_erase(state.game.full_deck, j);
            break;
          }
        }

        for (uint8_t j = 0; j < cvector_size(state.game.hand.cards); j++) {
          if (compare_cards(selected_cards[i], &state.game.hand.cards[j])) {
            cvector_erase(state.game.hand.cards, j);
            break;
          }
        }
      }
      break;
    case TAROT_DEATH:
      cvector_for_each(state.game.full_deck, Card, card) {
        if (compare_cards(selected_cards[0], card)) {
          *(selected_cards[0]) = *(selected_cards[1]);
          *card = *(selected_cards[1]);
          card->selected = 0;
          break;
        }
      }
      break;
    case TAROT_TEMPERANCE:
      // TODO Add this when selling items will be added
      break;
    case TAROT_JUDGEMENT:
      if (cvector_size(state.game.jokers.cards) >= state.game.jokers.size) break;
      cvector_push_back(state.game.jokers.cards, JOKERS[rand() % JOKER_COUNT]);
      break;

    case TAROT_HIGH_PRIESTESS:
      tarot_create_consumable(CONSUMABLE_PLANET);
      break;
    case TAROT_EMPEROR:
      tarot_create_consumable(CONSUMABLE_TAROT);
      break;

    case TAROT_LOVERS:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_WILD);
      break;
    case TAROT_CHARIOT:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_STEEL);
      break;
    case TAROT_JUSTICE:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_GLASS);
      break;
    case TAROT_DEVIL:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_GOLD);
      break;
    case TAROT_TOWER:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_STONE);
      break;
    case TAROT_MAGICIAN:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_LUCKY);
      break;
    case TAROT_EMPRESS:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_MULT);
      break;
    case TAROT_HIEROPHANT:
      tarot_change_enhancement(selected_cards, selected_count, ENHANCEMENT_BONUS);
      break;

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
  }

  if (max_selected_count != 0) deselect_all_cards();
  return 1;
}
