#include "utils.h"

#include "game.h"
#include "renderer.h"
#include "state.h"
#include "text.h"

CustomElementData create_spread_item_element(NavigationSection section, uint8_t i) {
  switch (section) {
    case NAVIGATION_HAND:
      return (CustomElementData){.type = CUSTOM_ELEMENT_CARD, .card = state.game.hand.cards[i]};

    case NAVIGATION_CONSUMABLES:
      return (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE, .consumable = state.game.consumables.items[i]};

    case NAVIGATION_JOKERS:
      return (CustomElementData){.type = CUSTOM_ELEMENT_JOKER, .joker = state.game.jokers.cards[i]};

    case NAVIGATION_SHOP_ITEMS: {
      ShopItem *item = &state.game.shop.items[i];
      switch (item->type) {
        case SHOP_ITEM_CARD:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CARD, .card = item->card};
        case SHOP_ITEM_JOKER:
          return (CustomElementData){.type = CUSTOM_ELEMENT_JOKER, .joker = item->joker};
        case SHOP_ITEM_PLANET:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE,
                                     .consumable = (Consumable){.type = CONSUMABLE_PLANET, .planet = item->planet}};
        case SHOP_ITEM_TAROT:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE,
                                     .consumable = (Consumable){.type = CONSUMABLE_TAROT, .tarot = item->tarot}};
      }
    }

    case NAVIGATION_SHOP_BOOSTER_PACKS:
      return (CustomElementData){.type = CUSTOM_ELEMENT_BOOSTER_PACK, .booster_pack = state.game.shop.booster_packs[i]};
      break;

    case NAVIGATION_BOOSTER_PACK: {
      BoosterPack *booster_pack = &state.game.booster_pack;
      BoosterPackContent *item = &booster_pack->content[i];

      switch (booster_pack->item.type) {
        case BOOSTER_PACK_STANDARD:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CARD, .card = item->card};
        case BOOSTER_PACK_BUFFON:
          return (CustomElementData){.type = CUSTOM_ELEMENT_JOKER, .joker = item->joker};
        case BOOSTER_PACK_CELESTIAL:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE,
                                     .consumable = (Consumable){.type = CONSUMABLE_PLANET, .planet = item->planet}};
        case BOOSTER_PACK_ARCANA:
          return (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE,
                                     .consumable = (Consumable){.type = CONSUMABLE_TAROT, .tarot = item->tarot}};
      }
      break;
    }
  }
}

void get_shop_item_tooltip_content(Clay_String *name, Clay_String *description, ShopItem *item) {
  switch (item->type) {
    case SHOP_ITEM_CARD:
      *name = get_full_card_name(item->card.suit, item->card.rank);
      append_clay_string(description, "+%d chips", item->card.chips);
      break;

    case SHOP_ITEM_JOKER:
      *name = (Clay_String){.chars = item->joker.name, .length = strlen(item->joker.name)};
      *description = (Clay_String){.chars = item->joker.description, .length = strlen(item->joker.description)};
      break;

    case SHOP_ITEM_PLANET:
      *name = (Clay_String){.chars = get_planet_card_name(item->planet),
                            .length = strlen(get_planet_card_name(item->planet))};
      uint16_t hand_union = 1 << item->planet;
      ScorePair upgrade = get_planet_card_base_score(hand_union);
      append_clay_string(description, "%s (+%u chips, +%0.lf mult)", get_poker_hand_name(hand_union), upgrade.chips,
                         upgrade.mult);
      break;

    case SHOP_ITEM_TAROT:
      append_clay_string(name, "Tarot %d", item->tarot);
      append_clay_string(description, "Does nothing");
      break;
  }
}

void get_nav_item_tooltip_content(Clay_String *name, Clay_String *description, NavigationSection section) {
  switch (section) {
    case NAVIGATION_HAND: {
      ShopItem item = {.type = SHOP_ITEM_CARD, .card = state.game.hand.cards[state.navigation.hovered]};
      get_shop_item_tooltip_content(name, description, &item);
      break;
    }

    case NAVIGATION_JOKERS: {
      ShopItem item = {.type = SHOP_ITEM_JOKER, .joker = state.game.jokers.cards[state.navigation.hovered]};
      get_shop_item_tooltip_content(name, description, &item);
      break;
    }

    case NAVIGATION_CONSUMABLES: {
      Consumable *consumable = &state.game.consumables.items[state.navigation.hovered];

      switch (consumable->type) {
        case CONSUMABLE_PLANET:
          get_shop_item_tooltip_content(name, description,
                                        &(ShopItem){.type = SHOP_ITEM_PLANET, .planet = consumable->planet});
          break;
        case CONSUMABLE_TAROT:
          get_shop_item_tooltip_content(name, description,
                                        &(ShopItem){.type = SHOP_ITEM_TAROT, .tarot = consumable->tarot});
          break;
      }
      break;
    }

    case NAVIGATION_SHOP_ITEMS: {
      ShopItem *item = &state.game.shop.items[state.navigation.hovered];
      get_shop_item_tooltip_content(name, description, item);
      break;
    }

    case NAVIGATION_SHOP_BOOSTER_PACKS: {
      BoosterPackItem *booster_pack = &state.game.shop.booster_packs[state.navigation.hovered];
      *name = get_full_booster_pack_name(booster_pack->size, booster_pack->type);
      append_clay_string(description, "Choose up to %d from %d", booster_pack->size == BOOSTER_PACK_MEGA ? 2 : 1,
                         booster_pack->size == BOOSTER_PACK_NORMAL ? 3 : 5);
      break;
    }

    case NAVIGATION_BOOSTER_PACK: {
      BoosterPackContent *item = &state.game.booster_pack.content[state.navigation.hovered];
      switch (state.game.booster_pack.item.type) {
        case BOOSTER_PACK_STANDARD:
          get_shop_item_tooltip_content(name, description, &(ShopItem){.type = SHOP_ITEM_CARD, .card = item->card});
          break;
        case BOOSTER_PACK_CELESTIAL:
          get_shop_item_tooltip_content(name, description,
                                        &(ShopItem){.type = SHOP_ITEM_PLANET, .planet = item->planet});
          break;
        case BOOSTER_PACK_BUFFON:
          get_shop_item_tooltip_content(name, description, &(ShopItem){.type = SHOP_ITEM_JOKER, .joker = item->joker});
          break;
        case BOOSTER_PACK_ARCANA:
          get_shop_item_tooltip_content(name, description, &(ShopItem){.type = SHOP_ITEM_TAROT, .tarot = item->tarot});
          break;
      }
      break;
    }
  }
}
