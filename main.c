#include <SDL.h>
#include <SDL_image.h>
#include <stdint.h>
#include <stdlib.h>

// #define __PSP__

#ifdef __PSP__

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define CARD_WIDTH CARD_SPRITE_WIDTH
#define CARD_HEIGHT CARD_SPRITE_HEIGHT

#else

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define CARD_WIDTH 96
#define CARD_HEIGHT 128

#endif

#define CARD_SPRITE_WIDTH 48
#define CARD_SPRITE_HEIGHT 64

SDL_Texture *cards_atlas;

typedef enum { SUIT_HEARTS, SUIT_DIAMONDS, SUIT_SPADES, SUIT_CLUBS } Suit;
typedef enum {
  RANK_ACE,
  RANK_TWO,
  RANK_THREE,
  RANK_FOUR,
  RANK_FIVE,
  RANK_SIX,
  RANK_SEVEN,
  RANK_EIGHT,
  RANK_NINE,
  RANK_TEN,
  RANK_JACK,
  RANK_QUEEN,
  RANK_KING
} Rank;

typedef enum {
  HAND_FLUSH_FIVE,
  HAND_FLUSH_HOUSE,
  HAND_FIVE_OF_KIND,
  HAND_STRAIGHT_FLUSH,
  HAND_FOUR_OF_KIND,
  HAND_FULL_HOUSE,
  HAND_FLUSH,
  HAND_STRAIGHT,
  HAND_THREE_OF_KIND,
  HAND_TWO_PAIR,
  HAND_PAIR,
  HAND_HIGH_CARD
} PokerHand;

typedef struct {
  Suit suit;
  Rank rank;
  // Since max 5 cards can be selected,
  // number 5 means not selected
  uint8_t selected;
} Card;

typedef struct {
  uint8_t size;
  uint8_t count;
  Card *cards;
} Hand, Deck;

void draw_card(SDL_Renderer *renderer, Suit suit, Rank rank, SDL_Rect *dst) {
  SDL_Rect src = {.x = rank * CARD_SPRITE_WIDTH,
                  .y = suit * CARD_SPRITE_HEIGHT,
                  .w = CARD_SPRITE_WIDTH,
                  .h = CARD_SPRITE_HEIGHT};
  SDL_RenderCopy(renderer, cards_atlas, &src, dst);
}

void draw_hand(SDL_Renderer *renderer, const Hand *hand, uint8_t hovered) {
  uint32_t hand_width = hand->count * (CARD_WIDTH - 16);
  for (uint8_t i = 0; i < hand->count; i++) {
    if (hovered == i) {
      continue;
    }

    SDL_Rect dst = {.x = SCREEN_WIDTH / 2 - hand_width / 2 +
                         (CARD_WIDTH - 16) * i,
                    .y = SCREEN_HEIGHT - CARD_HEIGHT - 16,
                    .w = CARD_WIDTH,
                    .h = CARD_HEIGHT};

    if (hand->cards[i].selected != 5) {
      dst.y -= 50;
    }

    draw_card(renderer, hand->cards[i].suit, hand->cards[i].rank, &dst);
  }

  draw_card(renderer, hand->cards[hovered].suit, hand->cards[hovered].rank,
            &(SDL_Rect){.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 +
                             (CARD_WIDTH - 16) * hovered - CARD_WIDTH * 0.1,
                        .y = SCREEN_HEIGHT - CARD_HEIGHT - 16 -
                             CARD_HEIGHT * 0.1 -
                             (hand->cards[hovered].selected != 5 ? 50 : 0),
                        .w = CARD_WIDTH * 1.2,
                        .h = CARD_HEIGHT * 1.2});
}

void toggle_card_select(Hand *hand, uint8_t hovered) {
  if (hand->cards[hovered].selected == 5) {
    uint8_t selected_count = 0;
    for (uint8_t i = 0; i < hand->count; i++) {
      if (hand->cards[i].selected != 5) {
        if (selected_count == 5) {
          return;
        }

        selected_count++;
      }
    }

    hand->cards[hovered].selected = selected_count;
    return;
  }

  uint8_t selected_order = hand->cards[hovered].selected;
  hand->cards[hovered].selected = 5;

  for (uint8_t i = 0; i < hand->count; i++) {
    if (hand->cards[i].selected > selected_order &&
        hand->cards[i].selected < 5) {
      hand->cards[i].selected--;
    }
  }
}

PokerHand evaluate_hand(Hand *hand) {
  uint8_t rank_counts[13] = {};
  uint8_t suit_counts[4] = {};

  // 2 of kind, 3 of kind, 4 of kind, 5 of kind
  uint8_t x_of_kind[4] = {};
  uint8_t selected_count = 0;
  uint8_t has_flush = 0;
  uint8_t has_straight = 0;

  Rank highest_card = RANK_TWO, lowest_card = RANK_ACE;

  for (uint8_t i = 0; i < hand->count; i++) {
    Card card = hand->cards[i];
    if (card.selected != 5) {
      selected_count += 1;

      if ((highest_card != RANK_ACE && card.rank > highest_card) ||
          card.rank == RANK_ACE) {
        highest_card = card.rank;
      }
      if ((card.rank != RANK_ACE && card.rank < lowest_card) ||
          lowest_card == RANK_ACE) {
        lowest_card = card.rank;
      }

      rank_counts[card.rank] += 1;
      suit_counts[card.suit] += 1;

      if (rank_counts[card.rank] > 1) {
        // this means there are duplicates in selected ranks,
        // so straight is not possible
        has_straight = 2;
        x_of_kind[rank_counts[card.rank] - 2] += 1;
        if (rank_counts[card.rank] > 2) {
          x_of_kind[rank_counts[card.rank] - 3] -= 1;
        }
      }

      if (suit_counts[card.suit] == 5) {
        has_flush = 1;
      }
    }
  }

  printf("Highest %d\tLowest %d\n", highest_card, lowest_card);

  if (has_straight == 0 && selected_count == 5 &&
      (highest_card - lowest_card == 4 ||
       (highest_card == RANK_ACE && 13 - lowest_card == 4))) {
    has_straight = 1;
  }

  if (has_flush == 1 && x_of_kind[5 - 2] > 0) {
    return HAND_FLUSH_FIVE;
  }

  if (has_flush == 1 && x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1) {
    return HAND_FLUSH_HOUSE;
  }

  if (x_of_kind[5 - 2] == 1) {
    return HAND_FIVE_OF_KIND;
  }

  if (has_flush == 1 && has_straight == 1) {
    return HAND_STRAIGHT_FLUSH;
  }

  if (x_of_kind[4 - 2] == 1) {
    return HAND_FOUR_OF_KIND;
  }

  if (x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1) {
    return HAND_FULL_HOUSE;
  }

  if (has_flush == 1) {
    return HAND_FLUSH;
  }

  if (has_straight == 1) {
    return HAND_STRAIGHT;
  }

  if (x_of_kind[3 - 2] == 1) {
    return HAND_THREE_OF_KIND;
  }

  if (x_of_kind[2 - 2] >= 2) {
    return HAND_TWO_PAIR;
  }

  if (x_of_kind[2 - 2] >= 1) {
    return HAND_PAIR;
  }

  return HAND_HIGH_CARD;
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

  SDL_Window *window =
      SDL_CreateWindow("Joker Poker", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Surface *cards_pixels = IMG_Load("../res/cards.png");
  cards_atlas = SDL_CreateTextureFromSurface(renderer, cards_pixels);
  SDL_FreeSurface(cards_pixels);

  srand(time(NULL));

  Deck deck = {
      .size = 52, .cards = (Card *)malloc(52 * sizeof(Card)), .count = 0};
  for (uint8_t i = 0; i < 52; i++) {
    deck.cards[deck.count] =
        (Card){.suit = i % 4, .rank = i % 13, .selected = 5};
    deck.count++;
  }

  Hand hand = {
      .size = 8, .cards = (Card *)malloc(8 * sizeof(Card)), .count = 8};
  hand.cards[0] = (Card){.rank = RANK_ACE, .suit = 1, .selected = 5};
  hand.cards[1] = (Card){.rank = RANK_TWO, .suit = 1, .selected = 5};
  // hand.cards[1] = (Card){.rank = RANK_TEN, .suit = 2, .selected = 5};
  hand.cards[2] = (Card){.rank = RANK_THREE, .suit = 1, .selected = 5};
  // hand.cards[2] = (Card){.rank = RANK_NINE, .suit = 1, .selected = 5};
  hand.cards[3] = (Card){.rank = RANK_FOUR, .suit = 1, .selected = 5};
  hand.cards[4] = (Card){.rank = RANK_FIVE, .suit = 1, .selected = 5};
  hand.cards[5] = (Card){.rank = RANK_JACK, .suit = 2, .selected = 5};
  hand.cards[6] = (Card){.rank = RANK_KING, .suit = 2, .selected = 5};
  hand.cards[7] = (Card){.rank = RANK_QUEEN, .suit = 2, .selected = 5};
  for (uint8_t i = 8; i < hand.size; i++) {
    hand.cards[hand.count] = deck.cards[rand() % 52];
    hand.count++;
  }
  uint8_t hovered = 0;

  int running = 1;
  SDL_Event event;
  uint32_t last_tick = 0;
  double delta = 0;
  while (running) {
    // Process input
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        // End the loop if the programs is being closed
        running = 0;
        break;
      case SDL_CONTROLLERDEVICEADDED:
        // Connect a controller when it is connected
        SDL_GameControllerOpen(event.cdevice.which);
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_RIGHT:
          if (hovered < hand.count - 1) {
            hovered += 1;
          }
          break;
        case SDLK_LEFT:
          if (hovered > 0) {
            hovered -= 1;
          }
          break;
        case SDLK_SPACE:
          toggle_card_select(&hand, hovered);
          printf("Selected hand: %d\n", evaluate_hand(&hand));
          break;
        }
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        switch (event.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_START:
          running = 0;
          break;
        case SDL_CONTROLLER_BUTTON_A:
          toggle_card_select(&hand, hovered);
          break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
          if (hovered < hand.count - 1) {
            hovered += 1;
          }
          break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
          if (hovered > 0) {
            hovered -= 1;
          }
          break;
        }
        break;
      }
    }

    uint32_t curr_tick = SDL_GetTicks();
    delta = (curr_tick - last_tick) / 1000.0;
    last_tick = curr_tick;

    // Clear the screen
    SDL_RenderClear(renderer);

    draw_hand(renderer, &hand, hovered);

    // Draw everything on a white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
