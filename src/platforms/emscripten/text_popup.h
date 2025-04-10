#pragma once

#include <SDL3/SDL.h>
#include <stddef.h>

enum TextPopupType {
    TextPopupType_INFO, // yellow colour, lasts 2s.
    TextPopupType_ERROR, // red colour, lasts 5s.
};

struct TextPopupEntry {
    char* str;
    Uint64 end;
    SDL_Color colour;
    struct TextPopupEntry* next;
    struct TextPopupEntry* prev;
};

struct TextPopup {
    struct TextPopupEntry* entries;
};

void text_popup_push(struct TextPopup* tp, enum TextPopupType type, const char* str);
void text_popup_push_arg(struct TextPopup* tp, enum TextPopupType type, SDL_PRINTF_FORMAT_STRING const char *fmt, ...) SDL_PRINTF_VARARG_FUNC(3);
void text_popup_render(struct TextPopup* tp, SDL_Renderer* renderer);
void text_popup_clear_all(struct TextPopup* tp);
