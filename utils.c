#include "utils.h"

#include "content/spectral.h"
#include "content/tarot.h"
#include "game.h"
#include "renderer.h"
#include "state.h"
#include "text.h"

CustomElementData create_spread_item_element(NavigationSection section, uint8_t i) {
  switch (section) {
    case NAVIGATION_NONE:
    case NAVIGATION_MAIN_MENU:
    case NAVIGATION_SELECT_DECK:
    case NAVIGATION_SELECT_STAKE:
    case NAVIGATION_SELECT_BLIND:
    case NAVIGATION_OVERLAY_MENU:
      return (CustomElementData){0};

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
        case SHOP_ITEM_SPECTRAL:
          return (CustomElementData){
              .type = CUSTOM_ELEMENT_CONSUMABLE,
              .consumable = (Consumable){.type = CONSUMABLE_SPECTRAL, .spectral = item->spectral}};
      }
    }

    case NAVIGATION_SHOP_VOUCHER:
      return (CustomElementData){.type = CUSTOM_ELEMENT_VOUCHER, .voucher = state.game.shop.vouchers[i]};

    case NAVIGATION_SHOP_BOOSTER_PACKS:
      return (CustomElementData){.type = CUSTOM_ELEMENT_BOOSTER_PACK, .booster_pack = state.game.shop.booster_packs[i]};

    case NAVIGATION_BOOSTER_PACK: {
      BoosterPack *booster_pack = &state.game.booster_pack;
      ShopItem *item = &booster_pack->content[i];

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
        case BOOSTER_PACK_SPECTRAL:
          return (CustomElementData){
              .type = CUSTOM_ELEMENT_CONSUMABLE,
              .consumable = (Consumable){.type = CONSUMABLE_SPECTRAL, .spectral = item->spectral}};
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
      *name =
          (Clay_String){.chars = get_tarot_card_name(item->tarot), .length = strlen(get_tarot_card_name(item->tarot))};
      *description = (Clay_String){.chars = get_tarot_card_description(item->tarot),
                                   .length = strlen(get_tarot_card_description(item->tarot))};
      break;

    case SHOP_ITEM_SPECTRAL:
      *name = (Clay_String){.chars = get_spectral_card_name(item->spectral),
                            .length = strlen(get_spectral_card_name(item->spectral))};
      *description = (Clay_String){.chars = get_spectral_card_description(item->spectral),
                                   .length = strlen(get_spectral_card_description(item->spectral))};
      break;
  }
}

void get_nav_item_tooltip_content(Clay_String *name, Clay_String *description, NavigationSection section) {
  switch (section) {
    case NAVIGATION_NONE:
    case NAVIGATION_MAIN_MENU:
    case NAVIGATION_SELECT_DECK:
    case NAVIGATION_SELECT_STAKE:
    case NAVIGATION_SELECT_BLIND:
    case NAVIGATION_OVERLAY_MENU:
      return;

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
        case CONSUMABLE_SPECTRAL:
          get_shop_item_tooltip_content(name, description,
                                        &(ShopItem){.type = SHOP_ITEM_SPECTRAL, .spectral = consumable->spectral});
      }
      break;
    }

    case NAVIGATION_SHOP_ITEMS: {
      ShopItem *item = &state.game.shop.items[state.navigation.hovered];
      get_shop_item_tooltip_content(name, description, item);
      break;
    }

    case NAVIGATION_SHOP_VOUCHER: {
      Voucher voucher = state.game.shop.vouchers[state.navigation.hovered];
      *name = (Clay_String){.chars = get_voucher_name(voucher), .length = strlen(get_voucher_name(voucher))};
      *description =
          (Clay_String){.chars = get_voucher_description(voucher), .length = strlen(get_voucher_description(voucher))};
      break;
    }

    case NAVIGATION_SHOP_BOOSTER_PACKS: {
      BoosterPackItem *booster_pack = &state.game.shop.booster_packs[state.navigation.hovered];
      *name = get_full_booster_pack_name(booster_pack->size, booster_pack->type);
      append_clay_string(description, "Choose %d of up to %d %s", booster_pack->size == BOOSTER_PACK_MEGA ? 2 : 1,
                         get_booster_pack_items_count(booster_pack),
                         get_booster_pack_description_suffix(booster_pack->type));
      break;
    }

    case NAVIGATION_BOOSTER_PACK:
      get_shop_item_tooltip_content(name, description, &state.game.booster_pack.content[state.navigation.hovered]);
      break;
  }
}

uint8_t get_item_price(NavigationSection section, uint8_t i) {
  switch (section) {
    case NAVIGATION_SHOP_ITEMS:
      return get_shop_item_price(&state.game.shop.items[i]);
    case NAVIGATION_SHOP_BOOSTER_PACKS:
      return get_booster_pack_price(&state.game.shop.booster_packs[i]);
    case NAVIGATION_SHOP_VOUCHER:
      return get_voucher_price(state.game.shop.vouchers[i]);
    default:
      return UINT8_MAX;
  }
}
