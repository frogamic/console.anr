/** \file cardView.c
 *  \author Dominic Shelton
 *  \date 8-6-15
 */

#include "cardView.h"


#define ANIMATION_DURATION 500

typedef struct {
    void (*destroy)(void*);
    void* context;
} Card;

CardView* CardView_create(Layer* parent) {
    CardView* cv = malloc(sizeof(CardView));
    if (cv) {
        cv->layerParent = parent;
        cv->layerNext = NULL;
        cv->layerCurrent = NULL;
        cv->animation = NULL;
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create CardView with parent %p", parent);
    }
    return cv;
}

Layer* CardView_add_card(CardView* cv, Direction d, GColor bg, void (*destroyCallback)(void*), void* context) {
    // y positions for above and below the screen.
    const int ypos[] = {144, -144};
    bool animating = false;
    GRect cardFrame = layer_get_frame(cv->layerParent);
    // Put the frame offscreen in the specified direction.
    cardFrame.origin.y = ypos[d];
    // Create the new layer with extra data for pointer to the context.
    Layer* layer = layer_create_with_data(cardFrame,sizeof(Card));

    // Ensure the layer was created.
    if (!layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create new card for CardView %p", cv);
        return NULL;
    }
    
    // Get the pointer to the data to store the callback and context.
    Card* card = layer_get_data(layer);
    card->destroy = destroyCallback;
    card->context = context;

    // If there is still an animation running destroy it.
    if (cv->animation)
    {
        animating = true;
        animation_destroy((Animation*)cv->animation);
    }
    // If there is already a next layer that had begun moving, move it into place.
    if (cv->layerNext && animating)
    {
        // Destroy old layer.
        CardView_destroy_card(cv->layerCurrent);
        // Replace the current layer with the new layer.
        cv->layerCurrent = cv->layerNext;
        // Reposition the new current layer which is not in place yet.
        layer_set_frame(cv->layerCurrent, layer_get_frame(cv->layerParent));
    }
    // Otherwise if the next layer hasn't begun moving onscreen destroy it.
    else if( cv->layerNext && !animating) {
        CardView_destroy_card(cv->layerNext);
    }

    // Finally replace the layer pointer and return it.
    cv->layerNext = layer;
    return layer;
}

static void animation_stop (Animation *animation, bool finished, void* context) {
    CardView* cv = (CardView*) context;
    // Destroy the previous current card.
    CardView_destroy_card(cv->layerCurrent);
    // Make the new card current.
    cv->layerCurrent = cv->layerNext;
    cv->layerNext = NULL;
#ifdef PBL_PLATFORM_APLITE
    animation_destroy((Animation*)animation);
#endif
}

int CardView_animate(CardView* cv) {
    // Check that an animation isn't already running.
    if (cv->animation) return 1;
    // Add the new layer as a child
    // If this is the first card then no animation is necessary, simply move next to current.
    if (!cv->layerCurrent) {
        cv->layerCurrent = cv->layerNext;
        cv->layerNext = NULL;
        // Add the new card layer to the parent layer.
        layer_add_child(cv->layerParent, cv->layerCurrent);
    }
    else {
        GRect target = layer_get_frame(cv->layerParent);
        layer_insert_below_sibling(cv->layerNext, cv->layerCurrent);
        cv->animation = property_animation_create_layer_frame(cv->layerNext, NULL, &target);
        animation_set_duration((Animation*)cv->animation, ANIMATION_DURATION);
        animation_set_handlers((Animation*)cv->animation, (AnimationHandlers) {
                .stopped = (AnimationStoppedHandler) animation_stop }, cv);
        animation_schedule((Animation*)cv->animation);
    }
    return 0;
}

void CardView_destroy_card(Layer* layer) {
    Card* c = layer_get_data(layer);
    // Just in case there is no data
    if (!c)
        return;
    // Call the destroy function passing it the context
    if (c->destroy)
        c->destroy(c->context);
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
    
}

