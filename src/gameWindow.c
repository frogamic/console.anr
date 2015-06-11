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
#define VALUETEXT_TOP_OFFSET 2
enum {VALUE_CLICKS = 0, VALUE_CREDS = 1, VALUE_CREDS_RECUR = 2};

static Window *window;
static GColor s_fg, s_bg;
static int value[2][3] = {{0, 0, 0}, {0, 5, 0}};
static int selectedValue;
static int s_clicks;
static TextLayer* valueLayer[3];
static char valueText[3][TEXT_LEN] = {{'\0'}};
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
    if (selectedValue == VALUE_CLICKS) {
        snprintf(valueText[selectedValue], TEXT_LEN, "%d of %d", value[0][selectedValue], value[1][selectedValue]);
    }
    else {
        snprintf(valueText[selectedValue], TEXT_LEN, "%d", value[0][selectedValue]);
    }
    text_layer_set_text(valueLayer[selectedValue], valueText[selectedValue]);
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
    app_log(APP_LOG_LEVEL_DEBUG, "fuckwit.c", 0, "window loading");
    window_set_background_color(window, s_bg);

    // Compute locations of selection boxes and textlayers.
    int screenheight = layer_get_frame(window_get_root_layer(window)).size.h;
    int padding = (screenheight - (VALUES * SELECTION_BOX_HEIGHT)) / (VALUES + 1);

    for (int i = 0; i< VALUES; i++)
    {
        selectionBox[i].origin.x = 0;
        selectionBox[i].origin.y = padding + (padding + SELECTION_BOX_HEIGHT) * i;
        selectionBox[i].size.w = layer_get_frame(window_get_root_layer(window)).size.w;
        selectionBox[i].size.h = SELECTION_BOX_HEIGHT;

        GRect textframe = selectionBox[i];
        textframe.origin.x += VALUETEXT_LEFT_OFFSET;
        textframe.size.w -= VALUETEXT_LEFT_OFFSET;
        textframe.origin.y += VALUETEXT_TOP_OFFSET;
        textframe.size.h -= VALUETEXT_TOP_OFFSET;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%d text layer at (%d,%d)", i, textframe.origin.x, textframe.origin.y);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "       size %dx%d", textframe.size.w, textframe.size.h);
        valueLayer[i] = text_layer_create(textframe);
        text_layer_set_text_color(valueLayer[i], s_fg);
        text_layer_set_background_color(valueLayer[i], GColorClear);
        text_layer_set_text(valueLayer[i],"error");
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(valueLayer[i]));
    };

    // Get number of clicks to start with
    selectedValue = 0;
    change_value(true, s_clicks);
}

static void window_unload(Window *window) {
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
    s_clicks = clicks;

    window_stack_push(window, true);
}

void gameWindow_deinit(void) {
    window_destroy(window);
}

