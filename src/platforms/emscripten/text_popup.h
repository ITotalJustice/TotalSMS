#pragma once

#include <SDL3/SDL.h>
#include <stddef.h>

enum TextPopupType {
    TextPopupType_INFO, // yellow colour, lasts 2s.
    TextPopupType_ERROR, // red colour, lasts 5s.
};

void text_popup_push(enum TextPopupType type, const char* str);
void text_popup_push_arg(enum TextPopupType type, SDL_PRINTF_FORMAT_STRING const char *fmt, ...) SDL_PRINTF_VARARG_FUNC(2);
void text_popup_render(SDL_Renderer* renderer);
void text_popup_clear_all(void);
