/** \file   gameWindow.c
 *  \author Dominic Shelton
 *  \date   8-6-2015
 */

#include "gameWindow.h"

#define TEXT_LEN 9
#define VALUES 3
#define LONG_CLICK_DURATION 700
#define SELECTION_BOX_HEIGHT 30
#define VALUETEXT_LEFT_OFFSET 40
#define VALUETEXT_TOP_OFFSET 40
enum {VALUE_CLICKS = 0, VALUE_CREDS = 1, VALUE_CREDS_RECUR = 2};

static Window *window;
static TextLayer *text_layer;
static GColor s_fg, s_bg;
static int value[2][3] = {{0, 0, 0}, {0, 5, 0}};
static int selectedValue;
static TextLayer* valueText[3];
static GRect selectionBox[3];

static void change_value(bool total, int amount) {
    value[0][selectedValue] += amount;
    // values can't go below 0.
    if (value[0][selectedValue] < 0) value[0][selectedValue] = 0;
    // Change the total value if applicable.
    if (total && selectedValue != VALUE_CREDS) {
        value[1][selectedValue] += amount;
        // Total values can't go below 0 either.
        if (value[1][selectedValue] < 0) value[1][selectedValue] = 0;
        // If the value goes above the total, do something.
        if (value[0][selectedValue] > value[1][selectedValue]) {
            if (selectedValue == VALUE_CREDS_RECUR)
                value[0][selectedValue] = value[1][selectedValue];
        }
    }
    char displayText[TEXT_LEN] = {'\0'};
    if (selectedValue == VALUE_CLICKS) {
        snprintf(displayText, TEXT_LEN, "%d of %d", value[0][selectedValue], value[0][selectedValue]);
    }
    else {
        snprintf(displayText, TEXT_LEN, "%d", value[0][selectedValue]);
    }
    text_layer_set_text(valueText[selectedValue], displayText);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    selectedValue = (selectedValue + 1) % VALUES;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    change_value(false, 1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    change_value(false, -1);
}

static void up_long_handler(ClickRecognizerRef recognizer, void *context) {
    change_value(true, 1);
}

static void down_long_handler(ClickRecognizerRef recognizer, void *context) {
    change_value(true, -1);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
    static bool pressed = false;
    if (pressed) window_stack_pop(true);
    else {
        text_layer_set_text(text_layer, "press again");
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
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(text_layer, "Press a button");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
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

    // Compute locations of selection boxes and textlayers.
    int screenheight = layer_get_frame(window_get_root_layer(window)).size.h;
    int padding = (screenheight - (VALUES * SELECTION_BOX_HEIGHT)) / (VALUES + 1);

    for (int i = 0; i< VALUES; i++)
    {
        selectionBox[i] = (GRect) {
            .origin.x = 0,
            .origin.y = padding + (padding + SELECTION_BOX_HEIGHT) * i,
            .size.w = layer_get_frame(window_get_root_layer(window)).size.w,
            .size.h = SELECTION_BOX_HEIGHT
        };
        GRect textframe = selectionBox[i];
        textframe.origin.x += VALUETEXT_LEFT_OFFSET;
        textframe.size.w -= VALUETEXT_LEFT_OFFSET;
        textframe.origin.y += VALUETEXT_TOP_OFFSET;
        textframe.size.h -= VALUETEXT_TOP_OFFSET;
        valueText[i] = text_layer_create(textframe);
    };

    // Get number of clicks to start with
    selectedValue = 0;
    change_value(true, clicks);

    window_stack_push(window, true);
}

void gameWindow_deinit(void) {
    window_destroy(window);
}

