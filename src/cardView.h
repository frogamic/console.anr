/** \file cardView.h
 *  \author Dominic Shelton
 *  \date 8-6-15
 */

#include <pebble.h>

typedef struct {
    Layer* layerParent;
    Layer* layerCurrent;
    Layer* layerNext;
    void (*destroyCurrent)(void*);
    void (*destroyNext)(void*);
    PropertyAnimation* animation;
} CardView;

typedef enum {FROM_ABOVE, FROM_BELOW} Direction;

/** Initialises the CardView and displays the initial card
 *  \param  parent  A pointer to the window in which to draw the cards.
 *  \return         A pointer to the CardView object created on the heap.
 */
CardView* CardView_create(Window* w);

/** Adds a new offscreen layer to the CardView, overwriting the existing offscreen layer if present.
 *  \param  cv              A pointer to the CardView to add the new card to
 *  \param  direction       The direction that the card should enter the screen (FROM_ABOVE or FROM_BELOW).
 *                          For the first layer to be added the direction will be ignored.
 *  \param  bg              The background color of the card.
 *  \param  destroyCallback A pointer to a function that is called when the card is deleted.
 *  \param  context         A pointer that is passed to the CardDestroyCallback function.
 *  \return     A pointer to the layer 
 */
Layer* CardView_add_card(CardView* cv, Direction d, GColor bg, void (*destroyCallback)(void*), void* context);

/** Destroys a layer initialised by CardView_add_card.
 *  \param layer    A layer created by CardView_add_card, calling on any other layer
 *                  has undefined results.
 */
void CardView_destroy_card(Layer* layer);

/** Animates the transition between layers, assuming no animation is already running.
 *  \param  cv  A pointer to the CardView to animate.
 *  \return     1 on failure, 0 on success.
 */
int CardView_animate(CardView* cv);

/** Destroys a CardView and all associated resources.
 *  \param  cv    A pointer to the CardView to destroy.
 */
void CardView_destroy(CardView* cv);

