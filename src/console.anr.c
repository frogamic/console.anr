#include <pebble.h>
#include "gameWindow.h"
#include "cardView.h"

#ifdef PBL_COLOR
#define FACTIONS 7
#else
#define FACTIONS 2
#endif
#define TEXT_Y 110
#define TEXT_HEIGHT 50
#define LOGO_Y 30
#define LOGO_HEIGHT 55

static Window *window;
static CardView* cardView;
static int selectedFaction = 0;

static GColor faction_get_fg(int faction) {
#ifdef PBL_COLOR
    switch (faction) {
        case 2: return GColorWhite;
        case 3: return GColorWhite;
        case 6: return GColorWhite;
    };
#endif
    return GColorBlack;
}

static GColor faction_get_color(int faction) {
#ifdef PBL_COLOR
    switch (faction) {
        case 0: return GColorRed;
        case 1: return GColorBlue;
        case 2: return GColorDarkCandyAppleRed;
        case 3: return GColorImperialPurple;
        case 4: return GColorChromeYellow;
        case 5: return GColorKellyGreen;
        case 6: return GColorMidnightGreen;
    };
#endif
    return GColorWhite;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    gameWindow_init (faction_get_color(selectedFaction), 4);
}

static void destroy_card(void* context) {
    TextLayer** layer = context;
    int i = 0;
    while (layer[i]) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "deleting sublayer %p", layer[i]);
        text_layer_destroy(layer[i]);
        ++i;
    }
    free(context);
}

static int make_card(CardView* cv, Direction d) {
#ifdef PBL_COLOR
    const char* factionLogos[] = {"\ue605", "\ue612", "\ue005", "\ue602", "\ue60b", "\ue603", "\ue607"};
    const char* factionNames[] = {"ANARCH", "CRIMINAL", "JINTEKI", "HAAS-\nBIOROID", "NBN", "SHAPER", "WEYLAND"};
#else
    const char* factionLogos[] = {"\ue605\ue612\ue613", "\ue005\ue602\ue60b\ue607"};
    const char* factionNames[] = {"Runner", "Corporation"};
#endif
    TextLayer** sublayers = malloc(sizeof(void*) * 3);
    Layer* layer = CardView_add_card(cv, d, faction_get_color(selectedFaction), destroy_card, sublayers);
    GRect frame = layer_get_frame(window_get_root_layer(window));
    // Create layers for text and logo.
    frame.origin.y = frame.size.h - TEXT_HEIGHT;
    frame.size.h = TEXT_HEIGHT;
    sublayers[0] = text_layer_create(frame);
    frame.origin.y = LOGO_Y;
    frame.size.h = LOGO_HEIGHT;
    sublayers[1] = text_layer_create(frame);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding sublayer %p", sublayers[0]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding sublayer %p", sublayers[1]);
    // Set both text layers to transparent and centered.
    text_layer_set_background_color(sublayers[0], GColorClear);
    text_layer_set_background_color(sublayers[1], GColorClear);
    text_layer_set_text_alignment(sublayers[0], GTextAlignmentCenter);
    text_layer_set_text_alignment(sublayers[1], GTextAlignmentCenter);
    // Set font and color.
    text_layer_set_text_color(sublayers[0], faction_get_fg(selectedFaction));
    text_layer_set_text_color(sublayers[1], faction_get_fg(selectedFaction));
    text_layer_set_font(sublayers[0], fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CIND_20)));
    text_layer_set_overflow_mode(sublayers[0], GTextOverflowModeWordWrap);
    text_layer_set_font(sublayers[1], fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FACTION_LOGOS_47)));
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
    make_card(cardView, FROM_ABOVE);
}

static void window_unload(Window *window) {
    CardView_destroy(cardView);
}

static void init(void) {
    window = window_create();
    cardView = CardView_create(window);
    window_set_click_config_provider_with_context(window, click_config_provider, cardView);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
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

