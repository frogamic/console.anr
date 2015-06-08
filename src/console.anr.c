#include <pebble.h>
#include "gameWindow.h"
#include "cardView.h"

#define FACTIONS 7
#define TEXT_Y 72
#define TEXT_HEIGHT 30
#define LOGO_Y 10
#define LOGO_HEIGHT 40

static Window *window;
static CardView* cardView;
static int selectedFaction = 0;
#ifdef PBL_COLOR
static const GColor factionColors[] = {GColorRed, GColorBlue, GColorDarkCandyAppleRed, GColorImperialPurple, GColorChromeYellow, GColorKellyGreen, GColorArmyGreen};
#endif

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef PBL_COLOR
    gameWindow_init (factionColors[selectedFaction], 4);
#endif
}

static void destroy_card(void* context) {
    TextLayer** layer = context;
    while (layer) {
        text_layer_destroy(*layer);
        ++layer;
    }
    free(context);
}

static int make_card(CardView* cv, Direction d) {
    const char* factionLogos[] = {"\ue605", "\ue612", "\ue005", "\ue602", "\ue60b", "\ue613", "\ue607"};
    const char* factionNames[] = {"Anarch", "Criminal", "Jinteki", "Haas-Bioroid", "NBN", "Shaper", "Weyland"};
    TextLayer** sublayers = malloc(sizeof(void*) * 3);
#ifdef PBL_COLOR
    Layer* layer = CardView_add_card(cv, d, factionColors[selectedFaction], destroy_card, sublayers);
#else
    Layer* layer = CardView_add_card(cv, d, GColorWhite, destroy_card, sublayers);
#endif
    GRect frame = layer_get_frame(layer);
    // Create layers for text and logo.
    frame.origin.y = TEXT_Y;
    frame.size.h = TEXT_HEIGHT;
    sublayers[0] = text_layer_create(frame);
    frame.origin.y = LOGO_Y;
    frame.size.h = LOGO_HEIGHT;
    sublayers[1] = text_layer_create(frame);
    // Set both text layers to transparent.
    text_layer_set_background_color(sublayers[0], GColorClear);
    text_layer_set_background_color(sublayers[1], GColorClear);
    // Set font of logo layer
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
    selectedFaction = (selectedFaction - 1) % FACTIONS;
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

