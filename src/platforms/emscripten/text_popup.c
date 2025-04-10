#include "text_popup.h"
#include <stdarg.h>

struct TypeEntry {
    SDL_Color colour;
    Uint32 time;
};

static const struct TypeEntry TYPE_ENTRIES[] = {
    [TextPopupType_INFO] = {
        .time = 2000, // 2s
        .colour = {0xFF, 0xFF, 0x30, SDL_ALPHA_OPAQUE}, // yellow
    },
    [TextPopupType_ERROR] = {
        .time = 5000, // 5s
        .colour = {0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE}, // red
    },
};

static void remove_entry(struct TextPopupEntry** head, struct TextPopupEntry* entry) {
    if ((*head) == entry) {
        (*head) = entry->next;
        if ((*head)) {
            (*head)->prev = NULL;
        }
    } else {
        entry->prev->next = entry->next;
        if (entry->next) {
            entry->next->prev = entry->prev;
        }
    }

    SDL_free(entry->str);
    SDL_free(entry);
}

void text_popup_push(struct TextPopup* tp, enum TextPopupType type, const char* str) {
    struct TextPopupEntry* entry = SDL_calloc(1, sizeof(*entry));
    entry->str = SDL_strdup(str);
    entry->end = SDL_GetTicks() + TYPE_ENTRIES[type].time;
    entry->colour = TYPE_ENTRIES[type].colour;

    if (!tp->entries) {
        tp->entries = entry;
    } else {
        struct TextPopupEntry* end = tp->entries;

        while (end->next) {
            end = end->next;
        }

        end->next = entry;
        entry->prev = end;
    }
}

void text_popup_push_arg(struct TextPopup* tp, enum TextPopupType type, const char *fmt, ...) {
    char buf[256];
    va_list va;
    va_start(va, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    text_popup_push(tp, type, buf);
}

void text_popup_render(struct TextPopup* tp, SDL_Renderer* renderer) {
    const Uint64 ms = SDL_GetTicks();
    const SDL_Color box_col = {0, 0, 0, 150};
    const float pad_x = 5;
    const float pad_y = 5;
    const float entry_pad = 2; // padding between entries.
    const float inc_y = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + entry_pad + (pad_y * 2.0F);

    float x = 0;
    float y = 0;

    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int w, h;
    SDL_GetCurrentRenderOutputSize(renderer, &w, &h);

    struct TextPopupEntry* entry = tp->entries;
    while (entry) {
        struct TextPopupEntry* temp = entry->next;

        if (ms >= entry->end) {
            remove_entry(&tp->entries, entry);
        } else {
            const float text_x = x + pad_x;
            const float text_y = y + pad_y;
            const float text_width = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(entry->str);
            const float text_height = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

            const SDL_FRect box_rect = {
                x, y,
                text_width + (pad_x * 2.0F),
                text_height + (pad_y * 2.0F)
            };

            // render box/
            SDL_SetRenderDrawColor(renderer, box_col.r, box_col.g, box_col.b, box_col.a);
            SDL_RenderFillRect(renderer, &box_rect);

            // render text.
            SDL_SetRenderDrawColor(renderer, entry->colour.r, entry->colour.g, entry->colour.b, entry->colour.a);
            SDL_RenderDebugText(renderer, text_x, text_y, entry->str);

            y += inc_y;

            if (y > h) {
                break;
            }
        }

        entry = temp;
    }

    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
}

void text_popup_clear_all(struct TextPopup* tp) {
    struct TextPopupEntry* entry = tp->entries;

    while (entry) {
        struct TextPopupEntry* temp = entry->next;
        remove_entry(&tp->entries, entry);
        entry = temp;
    }
}
