/** \file   gameWindow.c
 *  \author Dominic Shelton
 *  \date   8-6-2015
 */

#include "fonts.h"
#include "gameWindow.h"

#define TEXT_LEN 9
#define VALUES 2
#define LONG_CLICK_DURATION 500
#define MAX_CLICKS 6
#define CLICKS_Y 19
#define CLICKS_RADIUS 10
#define CLICKS_THICKNESS 3
#define CLICKS_FILL_RADIUS 5
#define CLICKS_SIZE ((CLICKS_RADIUS * 2) + 4)
#define CLICKS_X_OFFSET 1
#define CREDITS_Y 49
#define CREDITS_SYMBOL_OFFSET 6
#define SCREEN_WIDTH 144
#define TURN_RECT GRect(0, 112, SCREEN_WIDTH, 30)
#ifdef PBL_PLATFORM_BASALT
#define SELECT_RECT_0 GRect(1, 14 + STATUS_BAR_LAYER_HEIGHT, SCREEN_WIDTH - 2, 30)
#define SELECT_RECT_1 GRect(1, 58 + STATUS_BAR_LAYER_HEIGHT, SCREEN_WIDTH - 2, 42)
#define EXIT_RECT GRect(1, 14 + STATUS_BAR_LAYER_HEIGHT, SCREEN_WIDTH - 2, 66)
#else
#define SELECT_RECT_0 GRect(1, 14, SCREEN_WIDTH - 2, 30)
#define SELECT_RECT_1 GRect(1, 58, SCREEN_WIDTH - 2, 42)
#define EXIT_RECT GRect(1, 14, SCREEN_WIDTH - 2, 66)
#endif
#define EXIT_OUT_RECT GRect(1, -66, SCREEN_WIDTH - 2, 66)
#define EXIT_ANIMATION_DURATION 1500
#define ROUNDING 6
#define SELECT_ANIMATION_DURATION 250
#define CREDSYM "\ue600"
enum {VALUE_CLICKS = 0, VALUE_CREDITS = 1};

static Layer* layerGraphics, * layerSelection;
static Window *window;
static GColor s_fg, s_bg;
#ifdef PBL_COLOR
static GColor s_highlight;
#endif
static int avClicks, totalClicks;
static int credits = 5;
static int turns = 1;
static int selectedValue = 0;
static char creditText[TEXT_LEN] = "5";
static char turnText[TEXT_LEN] = "TURN 1";
static GRect selectionFrame[VALUES];
static PropertyAnimation* animationExiting = NULL;
#ifdef PBL_PLATFORM_BASALT
static StatusBarLayer* statusBar;
#endif

static void draw_click(GContext* ctx, bool filled, bool perm, int y, int x) {
    GPoint p = GPoint(x + CLICKS_RADIUS, y + CLICKS_RADIUS);
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

static void exit_update_proc(Layer* layer, GContext* ctx) {
    GRect rect = layer_get_frame(layer);
    rect.origin = GPointZero;
    graphics_context_set_fill_color(ctx, s_bg);
    graphics_context_set_text_color(ctx, s_fg);
    graphics_context_set_stroke_color(ctx, s_fg);
    graphics_fill_rect(ctx, rect, ROUNDING, GCornersAll);
    graphics_draw_round_rect(ctx, rect, ROUNDING);
    graphics_draw_text(ctx, "PRESS AGAIN TO EXIT", fontCINDSmall, rect,
            GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_credit_text(GContext* ctx, const char* credits, int y) {
    GRect credFrame, symFrame;
    credFrame.size = graphics_text_layout_get_content_size(
        credits, fontCINDLarge, GRect(0, 0, SCREEN_WIDTH, 50),
        GTextOverflowModeFill, GTextAlignmentLeft);
    symFrame.size = graphics_text_layout_get_content_size(
        CREDSYM, fontSymbolSmall, GRect(0, 0, SCREEN_WIDTH, 50),
        GTextOverflowModeFill, GTextAlignmentLeft);
    int credwidth = credFrame.size.w;
    int symwidth = symFrame.size.w;
    int halfwidth = (symwidth + credwidth) / 2;
    credFrame.origin.x = (SCREEN_WIDTH / 2) - halfwidth;
    symFrame.origin.x = (SCREEN_WIDTH / 2) + (halfwidth - symwidth);
    symFrame.origin.y = y + CREDITS_SYMBOL_OFFSET;
    credFrame.origin.y = y;
    graphics_context_set_text_color(ctx, s_fg);
    graphics_context_set_fill_color(ctx, GColorClear);
    graphics_draw_text(ctx, credits, fontCINDLarge, credFrame,
            GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, CREDSYM, fontSymbolSmall, symFrame,
            GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void selection_update_proc(Layer* layer, GContext* ctx) {
    GRect rect = layer_get_frame(layer);
    rect.origin = GPointZero;
#ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, s_highlight);
    graphics_fill_rect(ctx, rect, ROUNDING, GCornersAll);
#endif
    graphics_context_set_stroke_color(ctx, s_fg);
    graphics_draw_round_rect(ctx, rect, ROUNDING);
}

static void main_update_proc(Layer* layer, GContext* ctx) {
    int t = (avClicks < totalClicks) ? totalClicks : avClicks;
    int x = (SCREEN_WIDTH - t * CLICKS_SIZE) / 2 + CLICKS_X_OFFSET;
    for (int i = 0; i < t; i++) {
        draw_click(ctx, avClicks > i, totalClicks > i, CLICKS_Y, x);
        x += CLICKS_SIZE;
    }
    draw_credit_text(ctx, creditText, CREDITS_Y);
    graphics_context_set_text_color(ctx, s_fg);
    graphics_draw_text(ctx, turnText, fontCINDSmall, TURN_RECT,
            GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void exit_animation_stopped(Animation* animation, bool finished, void* data) {
#ifdef PBL_PLATFORM_APLITE
    if (finished) {
        animation_destroy(animation);
    }
#endif
    layer_destroy((Layer*)data);
    animationExiting = NULL;
}

static void select_animation_stopped(Animation* animation, bool finished, void* context) {
#ifdef PBL_PLATFORM_APLITE
    if (finished) {
        animation_destroy(animation);
    }
#endif
    *(PropertyAnimation**)context = NULL;
}
static void reprint_text (bool markDirty) {
    animationExiting = false;
    static int lastCredits = 0;
    static int lastTurns = 0;
    if (credits != lastCredits) {
        snprintf(creditText, TEXT_LEN, "%u", credits);
        lastCredits = credits;
        if (markDirty)
            layer_mark_dirty(layerGraphics);
    }
    if (turns != lastTurns) {
        snprintf(turnText, TEXT_LEN, "TURN %u", turns);
        lastTurns = turns;
        if (markDirty)
            layer_mark_dirty(layerGraphics);
    }
}

static void new_turn(void) {
    avClicks = totalClicks;
    turns++;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (selectedValue == VALUE_CLICKS) {
        avClicks++;
        avClicks = (avClicks < MAX_CLICKS) ? avClicks : MAX_CLICKS;
        layer_mark_dirty(layerGraphics);
    }
    else if (selectedValue == VALUE_CREDITS) {
        credits++;
    }
    reprint_text(true);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (selectedValue == VALUE_CLICKS) {
        avClicks--;
        if (avClicks <= 0) {
            avClicks = 0;
            new_turn();
        }
        layer_mark_dirty(layerGraphics);
    }
    else if (selectedValue == VALUE_CREDITS) {
        credits--;
        credits = (credits < 0) ? 0 : credits;
    }
    reprint_text(true);
}

static void up_long_handler(ClickRecognizerRef recognizer, void *context) {
    if (selectedValue == VALUE_CLICKS) {
        totalClicks++;
    }
    else if (selectedValue == VALUE_CREDITS) {
        credits += 4;
    }
    up_click_handler(recognizer, context);
}

static void down_long_handler(ClickRecognizerRef recognizer, void *context) {
    if (selectedValue == VALUE_CLICKS) {
        totalClicks--;
        totalClicks = (totalClicks > 1) ? totalClicks : 1;
    }
    else if (selectedValue == VALUE_CREDITS) {
        credits -= 4;
    }
    down_click_handler(recognizer, context);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    selectedValue = (selectedValue + 1) % VALUES;
    static PropertyAnimation* animation = NULL;

    if (animation != NULL)
       animation_destroy((Animation*)animation);
    animation = property_animation_create_layer_frame(layerSelection, NULL,
            &selectionFrame[selectedValue]);
    animation_set_curve((Animation*) animation, AnimationCurveEaseOut);
    animation_set_duration((Animation*) animation, SELECT_ANIMATION_DURATION);
    animation_set_handlers((Animation*) animation, (AnimationHandlers){
            .stopped = select_animation_stopped}, &animation);
    animation_schedule((Animation*) animation);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (animationExiting) {
        gameWindow_deinit();
    }
    else {
        Layer* layer = layer_create(EXIT_RECT);
        GRect rect = EXIT_OUT_RECT;
        layer_set_update_proc(layer, exit_update_proc);
        layer_add_child(window_get_root_layer(window), layer);
        animationExiting = property_animation_create_layer_frame(layer, NULL,
                &rect);
        animation_set_curve((Animation*) animationExiting, AnimationCurveEaseIn);
        animation_set_duration((Animation*) animationExiting, EXIT_ANIMATION_DURATION);
        animation_set_handlers((Animation*) animationExiting, (AnimationHandlers){
                .stopped = exit_animation_stopped}, layer);
        animation_schedule((Animation*) animationExiting);
    }
}

static void click_config_provider_tutorial(void *context) {
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
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
    Layer* root = window_get_root_layer(window);
    window_set_background_color(window, s_bg);
    GRect windowFrame = layer_get_frame(root);

#ifdef PBL_PLATFORM_BASALT
    statusBar = status_bar_layer_create();
    layer_add_child(root, status_bar_layer_get_layer(statusBar));
    windowFrame.origin.y += STATUS_BAR_LAYER_HEIGHT;
#endif

    // Add the graphics and selection layers
    layerGraphics = layer_create(windowFrame);
    layerSelection = layer_create(selectionFrame[0]);
    layer_set_update_proc(layerGraphics, main_update_proc);
    layer_set_update_proc(layerSelection, selection_update_proc);
    layer_add_child(root, layerSelection);
    layer_add_child(root, layerGraphics);
}

static void window_unload(Window *window) {
    if (animationExiting)
        property_animation_destroy(animationExiting);
    layer_destroy(layerGraphics);
    layer_destroy(layerSelection);
#ifdef PBL_PLATFORM_BASALT
    status_bar_layer_destroy(statusBar);
#endif
}

#ifdef PBL_COLOR
inline GColor get_highlight(GColor color) {
    switch (color.argb) {
        case GColorRedARGB8: return GColorSunsetOrange;
        case GColorBlueARGB8: return GColorDukeBlue;
        case GColorDarkCandyAppleRedARGB8: return GColorBulgarianRose;
        case GColorImperialPurpleARGB8: return GColorOxfordBlue;
        case GColorChromeYellowARGB8: return GColorYellow;
        case GColorKellyGreenARGB8: return GColorInchworm;
        case GColorMidnightGreenARGB8: return GColorDarkGray;
        case GColorWhiteARGB8: return GColorLightGray;
        case GColorBlackARGB8: return GColorDarkGray;
    }
    return GColorBlack;
}
#endif

void gameWindow_init(GColor bg, GColor fg, int clicks) {
    window = window_create();
    if (clicks == 0)
        window_set_click_config_provider(window, click_config_provider_tutorial);
    else
        window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    selectionFrame[0] = SELECT_RECT_0;
    selectionFrame[1] = SELECT_RECT_1;

    // Get colors.
    s_fg = fg;
    s_bg = bg;
#ifdef PBL_COLOR
    s_highlight = get_highlight(s_bg);
#endif

    selectedValue = VALUE_CLICKS;

    avClicks = totalClicks = clicks;
    credits = 5;
    turns = 1;

    reprint_text(false);

    APP_LOG(APP_LOG_LEVEL_INFO, "Done initializing, pushed window: %p", window);

    window_stack_push(window, true);
}

void gameWindow_deinit(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "De-initializing, destroying window: %p", window);
    window_stack_remove(window, true);
    window_destroy(window);
}

