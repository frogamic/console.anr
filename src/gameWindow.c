/** \file   gameWindow.c
 *  \author Dominic Shelton
 *  \date   8-6-2015
 */

#include "gameWindow.h"

#define TEXT_LEN 9
#define VALUES 2
#define LONG_CLICK_DURATION 700
#define CLICKS_Y 19
#define CLICKS_RADIUS 10
#define CLICKS_THICKNESS 3
#define CLICKS_FILL_RADIUS 5
#define CREDITS_Y 49
#define CREDITS_SYMBOL_OFFSET 6
#define SCREENWIDTH 144
#define TURNRECT (GRect){.size.w=SCREENWIDTH, .size.h=30, .origin.x=0, .origin.y=112}
#define SELECTION_ROUNDING 6
#define CREDSYM "\ue600"
enum {VALUE_CLICKS = 0, VALUE_CREDS = 1, VALUE_CREDS_RECUR = 2};

static Layer* layerGraphics, * layerSelection;
static Window *window;
static GColor s_fg, s_bg;
static int avClicks, totalClicks;
static int credits = 5;
static int turns = 1;
static int selectedValue = 0;
static GFont font_cind_small, font_cind_large, font_symbols;
static char creditText[TEXT_LEN] = "123";
static char turnText[TEXT_LEN] = "TURN 6";
static GRect selectionFrame[] = {
    (GRect){
        .origin.x = 1,
        .origin.y = 14,
        .size.w = SCREENWIDTH - 2,
        .size.h = 30,
    },
    (GRect){
        .origin.x = 1,
        .origin.y = 58,
        .size.w = SCREENWIDTH - 2,
        .size.h = 42,
    },
};

static void draw_click(GContext* ctx, bool filled, bool perm, int y, int x) {
    GPoint p = {.x = x + CLICKS_RADIUS, .y = y + CLICKS_RADIUS};
    if (perm) {
#ifdef PBL_PLATFORM_APLITE
        graphics_context_set_fill_color(ctx, s_fg);
        graphics_fill_circle(ctx, p, CLICKS_RADIUS);
        graphics_context_set_fill_color(ctx, s_bg);
        graphics_fill_circle(ctx, p, CLICKS_RADIUS - CLICKS_THICKNESS);
#else
        graphics_context_set_stroke_color(ctx, s_fg);
        graphics_context_set_stroke_width(ctx, CLICKS_THICKNESS);
        graphics_draw_circle(ctx, p, CLICKS_RADIUS - (CLICKS_THICKNESS / 2));
#endif
    }
    if (filled) {
        graphics_context_set_fill_color(ctx, s_fg);
        graphics_fill_circle(ctx, p, CLICKS_FILL_RADIUS);
    }
}

static void draw_credit_text(GContext* ctx, const char* credits, int y) {
    GRect credFrame, symFrame;
    credFrame.size = graphics_text_layout_get_content_size(
        credits, font_cind_large, (GRect){.size.w = SCREENWIDTH, .size.h = 50},
        GTextOverflowModeFill, GTextAlignmentLeft);
    symFrame.size = graphics_text_layout_get_content_size(
        CREDSYM, font_symbols, (GRect){.size.w = SCREENWIDTH, .size.h = 50},
        GTextOverflowModeFill, GTextAlignmentLeft);
    int credwidth = credFrame.size.w;
    int symwidth = symFrame.size.w;
    int halfwidth = (symwidth + credwidth) / 2;
    credFrame.origin.x = (SCREENWIDTH / 2) - halfwidth;
    symFrame.origin.x = (SCREENWIDTH / 2) + (halfwidth - symwidth);
    symFrame.origin.y = y + CREDITS_SYMBOL_OFFSET;
    credFrame.origin.y = y;
    graphics_context_set_text_color(ctx, s_fg);
    graphics_context_set_fill_color(ctx, GColorClear);
    graphics_draw_text(ctx, credits, font_cind_large, credFrame,
            GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, CREDSYM, font_symbols, symFrame,
            GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void selection_update_proc(Layer* layer, GContext* ctx) {
    graphics_context_set_stroke_color(ctx, s_fg);
    GRect rect = layer_get_bounds(layer);
    graphics_draw_round_rect(ctx, rect, SELECTION_ROUNDING);
}

static void main_update_proc(Layer* layer, GContext* ctx) {
    draw_click(ctx, true, true, CLICKS_Y, 38);
    draw_click(ctx, true, true, CLICKS_Y, 62);
    draw_click(ctx, true, true, CLICKS_Y, 86);
    draw_credit_text(ctx, creditText, CREDITS_Y);
    graphics_context_set_text_color(ctx, s_fg);
    graphics_draw_text(ctx, turnText, font_cind_small, TURNRECT,
            GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    selectedValue = (selectedValue + 1) % VALUES;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_long_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_long_handler(ClickRecognizerRef recognizer, void *context) {
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
    static bool pressed = false;
    if (pressed) window_stack_pop(true);
    else {
        pressed = true;
    }
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_long_click_subscribe(BUTTON_ID_UP, LONG_CLICK_DURATION, up_long_handler, NULL);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_long_click_subscribe(BUTTON_ID_DOWN, LONG_CLICK_DURATION, down_long_handler, NULL);
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void window_load(Window *window) {
    window_set_background_color(window, s_bg);

    // Load the fonts.
    font_cind_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CIND_20));
    font_cind_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CIND_46));
    font_symbols = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GAME_SYMBOLS_40));

    // Add the graphics and selection layers
    layerGraphics = layer_create(layer_get_frame(window_get_root_layer(window)));
    layerSelection = layer_create(selectionFrame[1]);
    layer_set_update_proc(layerGraphics, main_update_proc);
    layer_set_update_proc(layerSelection, selection_update_proc);
    layer_add_child(window_get_root_layer(window), layerGraphics);
    layer_add_child(window_get_root_layer(window), layerSelection);
}

static void window_unload(Window *window) {
    fonts_unload_custom_font(font_cind_small);
    fonts_unload_custom_font(font_cind_large);
    fonts_unload_custom_font(font_symbols);
    layer_destroy(layerGraphics);
    layer_destroy(layerSelection);
}

void gameWindow_init(GColor bg, GColor fg, int clicks) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    // Get colors.
    s_fg = fg;
    s_bg = bg;

    window_stack_push(window, true);
}

void gameWindow_deinit(void) {
    window_destroy(window);
}

