/** \file cardView.c
 *  \author Dominic Shelton
 *  \date 8-6-15
 */

#include "cardView.h"

#define ANIMATION_DURATION 250

typedef struct {
    void (*destroy)(void*);
    void* context;
    GColor bg;
} Card;

static void fill_update_proc(Layer* layer, GContext* ctx) {
    Card* c = layer_get_data(layer);
    graphics_context_set_fill_color(ctx, c->bg);
    graphics_fill_rect(ctx,layer_get_frame(layer), 0, GCornerNone);
}

static void animation_stop (Animation *animation, bool finished, void* context) {
    CardView* cv = (CardView*) context;
    // Destroy the previous current card.
    CardView_destroy_card(cv->layerCurrent);
    // Make the new card current.
    cv->layerCurrent = cv->layerNext;
    cv->layerNext = NULL;
    // If the animation is not finished, the new layer must be moved to the correct position.
    if (!finished) {
        layer_set_frame(cv->layerCurrent, layer_get_frame(cv->layerParent));
    }
    // Destroy the animation only if it finished naturally on aplite
#ifdef PBL_PLATFORM_APLITE
    if (finished)
        animation_destroy((Animation*)animation);
#endif
    cv->animation = NULL;
}

CardView* CardView_create(Window* w) {
    CardView* cv = malloc(sizeof(CardView));
    if (cv) {
        cv->layerParent = window_get_root_layer(w);
        cv->layerNext = NULL;
        cv->layerCurrent = NULL;
        cv->animation = NULL;
    }
    return cv;
}

Layer* CardView_add_card(CardView* cv, Direction d, GColor bg, void (*destroyCallback)(void*), void* context) {
    // y positions for above and below the screen.
    GRect cardFrame = layer_get_frame(cv->layerParent);
    const int ypos[] = {0 - cardFrame.size.h, cardFrame.size.h};
    // Put the frame offscreen in the specified direction.
    cardFrame.origin.y = ypos[d];
    // Create the new layer with extra data for pointer to the context.
    Layer* layer = layer_create_with_data(cardFrame,sizeof(Card));

    // Ensure the layer was created.
    if (!layer) {
        return NULL;
    }
    
    // Get the pointer to the data to store the callback and context.
    Card* card = layer_get_data(layer);
    card->destroy = destroyCallback;
    card->context = context;
    // Set the draw method and bg color of the new card.
    card->bg = bg;
    layer_set_update_proc(layer, (LayerUpdateProc) fill_update_proc);

    // If there is still an animation running destroy it.
    if (cv->animation)
    {
        animation_destroy((Animation*)cv->animation);
    }
    // Otherwise if the next layer hasn't begun moving onscreen destroy it.
    else if( cv->layerNext) {
        CardView_destroy_card(cv->layerNext);
    }

    // Finally replace the layer pointer and return it.
    cv->layerNext = layer;
    return layer;
}

int CardView_animate(CardView* cv) {
    // Check that an animation isn't already running.
    if (cv->animation) return 1;
    // Get the target position for the new card.
    GRect target = layer_get_frame(cv->layerParent);
    // Add the new layer as a child
    // If this is the first card then no animation is necessary, simply move next to current.
    if (!cv->layerCurrent) {
        cv->layerCurrent = cv->layerNext;
        cv->layerNext = NULL;
        // Set the new card's frame to window size.
        layer_set_frame(cv->layerCurrent, target);
        // Add the new card layer to the parent layer.
        layer_add_child(cv->layerParent, cv->layerCurrent);
    }
    else {
        layer_insert_below_sibling(cv->layerNext, cv->layerCurrent);
        cv->animation = property_animation_create_layer_frame(cv->layerNext, NULL, &target);
        animation_set_duration((Animation*)cv->animation, ANIMATION_DURATION);
        animation_set_handlers((Animation*)cv->animation, (AnimationHandlers) {
                .stopped = (AnimationStoppedHandler) animation_stop }, cv);
        layer_add_child(cv->layerParent, cv->layerNext);
        animation_schedule((Animation*)cv->animation);
    }
    return 0;
}

void CardView_destroy_card(Layer* layer) {
    Card* c = layer_get_data(layer);
    // Just in case there is no data
    if (!c) {
        layer_destroy(layer);
        return;
    }
    // Call the destroy function passing it the context
    if (c->destroy)
    {
        c->destroy(c->context);
    }
    layer_destroy(layer);
}

void CardView_destroy(CardView* cv) {
    if (cv->animation)
    {
        animation_destroy((Animation*)cv->animation);
    }
    if(cv->layerNext) {
        CardView_destroy_card(cv->layerNext);
    }
    if(cv->layerCurrent) {
        CardView_destroy_card(cv->layerCurrent);
    }
    free(cv);
}

