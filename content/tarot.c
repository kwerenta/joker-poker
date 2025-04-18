#include "tarot.h"

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
