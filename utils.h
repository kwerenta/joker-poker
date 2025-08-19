#ifndef UTILS_H
#define UTILS_H

#include "renderer.h"
#include "state.h"

CustomElementData create_spread_item_element(NavigationSection section, uint8_t i);

void get_shop_item_tooltip_content(Clay_String *name, Clay_String *description, ShopItem *item);
void get_nav_item_tooltip_content(Clay_String *name, Clay_String *description, NavigationSection section);

uint8_t get_item_price(NavigationSection section, uint8_t i);

#endif
