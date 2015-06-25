/** \file   fonts.h
 *  \author Dominic Shelton
 *  \date   26-6-2015
 */

#include <pebble.h>

GFont fontCINDSmall, fontCINDLarge, fontSymbolSmall, fontSymbolLarge;

inline void fonts_load(void) {
    fontSymbolSmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GAME_SYMBOLS_40));
    fontSymbolLarge = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GAME_SYMBOLS_46));
    fontCINDSmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CIND_20));
    fontCINDLarge = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CIND_46));
}

inline void fonts_unload(void) {
    fonts_unload_custom_font(fontSymbolSmall);
    fonts_unload_custom_font(fontSymbolLarge);
    fonts_unload_custom_font(fontCINDSmall);
    fonts_unload_custom_font(fontCINDLarge);
}

