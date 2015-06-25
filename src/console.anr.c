/** \file   console.anr.c
 *  \author Dominic Shelton
 *  \date   6-6-2015
 */

#include <pebble.h>
#include "gameWindow.h"
#include "fonts.h"
#include "cardView.h"

#ifdef PBL_COLOR
#define FACTIONS 8
#else
#define FACTIONS 3
#endif
#define LOGO_Y 25
#define LOGO_HEIGHT 100
#define TEXT_HEIGHT 50
#define HB_TEXTHEIGHT 20

#ifdef PBL_COLOR
enum {CORP, RUNNER, ANARCH, CRIMINAL, JINTEKI, HAAS, NBN, SHAPER, WEYLAND, TUTORIAL};
#else
enum {CORP, RUNNER, TUTORIAL};
#endif
static Window *window;
static CardView* cardView;
static int selectedFaction = 0;

static GColor faction_get_fg(int faction) {
    switch (faction) {
        case CORP: return GColorWhite;
#ifdef PBL_COLOR
        case CRIMINAL: return GColorWhite;
        case JINTEKI: return GColorWhite;
        case HAAS: return GColorWhite;
        case WEYLAND: return GColorWhite;
        case TUTORIAL: return GColorWhite;
#endif
        default: return GColorBlack;
    };
}

static GColor faction_get_color(int faction) {
    switch (faction) {
        case CORP: return GColorBlack;
#ifdef PBL_COLOR
        case ANARCH: return GColorRed;
        case CRIMINAL: return GColorBlue;
        case JINTEKI: return GColorDarkCandyAppleRed;
        case HAAS: return GColorImperialPurple;
        case NBN: return GColorChromeYellow;
        case SHAPER: return GColorKellyGreen;
        case WEYLAND: return GColorMidnightGreen;
        case TUTORIAL: return GColorDarkGray;
#endif
        default: return GColorWhite;
    };
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef PBL_COLOR
    int clicks[] = {3, 4, 4, 4, 3, 3, 3, 4, 3, 0};
#else
    int clicks[] = {3, 4, 0};
#endif
    gameWindow_init (faction_get_color(selectedFaction), faction_get_fg(selectedFaction), clicks[selectedFaction]);
}

static void destroy_card(void* context) {
    TextLayer** layer = context;
    int i = 0;
    while (layer[i]) {
        text_layer_destroy(layer[i]);
        ++i;
    }
    free(context);
}

static int make_card(CardView* cv, Direction d) {
#ifdef PBL_COLOR
    const char* factionLogos[] = {"\ue005\ue602\n\ue60b\ue607", "\ue605\ue612\ue613", "\ue605", "\ue612", "\ue005", "\ue602", "\ue60b", "\ue613", "\ue607", "\ue611\ue600"};
    const char* factionNames[] = {"CORP", "RUNNER", "ANARCH", "CRIMINAL", "JINTEKI", "HAAS-\nBIOROID", "NBN", "SHAPER", "WEYLAND", "TUTORIAL"};
#else
    const char* factionLogos[] = {"\ue005\ue602\n\ue60b\ue607", "\ue605\ue612\ue613", "\ue611\ue600"};
    const char* factionNames[] = {"CORP", "RUNNER", "TUTORIAL"};
#endif
    TextLayer** sublayers = malloc(sizeof(void*) * 3);
    Layer* layer = CardView_add_card(cv, d, faction_get_color(selectedFaction), destroy_card, sublayers);
    GRect frame = layer_get_frame(window_get_root_layer(window));
    // Create layers for text and logo.
    frame.origin.y = frame.size.h - TEXT_HEIGHT;
    frame.size.h = TEXT_HEIGHT;
#ifdef PBL_COLOR
    // If the text is HB, move up for the two lines
    if (selectedFaction == HAAS) frame.origin.y -= HB_TEXTHEIGHT;
#endif
    // If the corp is selected move the text down to make room for the logos.
    if (selectedFaction == CORP) frame.origin.y += LOGO_Y / 2;
    sublayers[0] = text_layer_create(frame);
    frame.origin.y = LOGO_Y;
    frame.size.h = LOGO_HEIGHT;
    // If the corp is selected move the logos up to make room for both lines.
    if (selectedFaction == CORP) frame.origin.y-= LOGO_Y / 2;
    sublayers[1] = text_layer_create(frame);
    // Set both text layers to transparent and centered.
    text_layer_set_background_color(sublayers[0], GColorClear);
    text_layer_set_background_color(sublayers[1], GColorClear);
    text_layer_set_text_alignment(sublayers[0], GTextAlignmentCenter);
    text_layer_set_text_alignment(sublayers[1], GTextAlignmentCenter);
    // Set font and color.
    text_layer_set_text_color(sublayers[0], faction_get_fg(selectedFaction));
    text_layer_set_text_color(sublayers[1], faction_get_fg(selectedFaction));
    text_layer_set_font(sublayers[0], fontCINDSmall);
    text_layer_set_overflow_mode(sublayers[0], GTextOverflowModeWordWrap);
    text_layer_set_font(sublayers[1], fontSymbolLarge);
    // Set name and logo in layers.
    text_layer_set_text(sublayers[0], factionNames[selectedFaction]);
    text_layer_set_text(sublayers[1], factionLogos[selectedFaction]);
    // Add the layers as sublayers of the card.
    layer_add_child(layer, (Layer*)sublayers[0]);
    layer_add_child(layer, (Layer*)sublayers[1]);
    // Null terminate the list of layers to be destroyed.
    sublayers[2] = NULL;
    return CardView_animate(cv);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    selectedFaction = (selectedFaction + FACTIONS - 1) % FACTIONS;
    make_card(cardView, FROM_ABOVE);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    selectedFaction = (selectedFaction + 1) % FACTIONS;
    make_card(cardView, FROM_BELOW);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
    fonts_load();
    cardView = CardView_create(window);
    make_card(cardView, FROM_ABOVE);
}

static void window_unload(Window *window) {
    fonts_unload();
    CardView_destroy(cardView);
}

static void init(void) {
    window = window_create();
    window_set_click_config_provider_with_context(window, click_config_provider, cardView);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    gameWindow_deinit();
    deinit();
}

