#include "rewind_bar.h"

struct RewindBar {
    int cursor;
    int count;
    bool enable;
};

static struct RewindBar g_bar;
static const float BAR_HEIGHT = 70;

static void rewind_bar_change_direction(int v) {
    const int result = g_bar.cursor + v;

    if (result >= 0 && result < g_bar.count) {
        g_bar.cursor = result;
    }
}

static bool render_entry(App* app, int index, const SDL_FRect* rect, const SDL_FRect* bar) {
    if (!rewind_get(app->rewind, index, app->rewind_buffer, app->rewind_buffer_size)) {
        return false;
    }

    if (index == g_bar.cursor) {
        const float outline_size = 2;
        SDL_FRect outline = *rect;
        outline.x -= outline_size;
        outline.y -= outline_size;
        outline.w += outline_size * 2;
        outline.h += outline_size * 2;

        SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(app->renderer, &outline);
    }

    char buf[32];
    // SDL_snprintf(buf, sizeof(buf), "%d", g_bar.count - 1 - index);
    SDL_snprintf(buf, sizeof(buf), "%.1fs", (double)(g_bar.count - 1 - index) * (double)app->rewind_keyframe_interval / 60.0);
    const float str_size = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(buf);
    const float center_x = (rect->x + rect->w / 2) - str_size / 2;
    const float center_y = bar->y + ((rect->y - bar->y) / 2) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE / 2);

    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(app->renderer, center_x, center_y, buf);

    emulator_update_texture_pixels(app, app->rewind_pixel_buffer);
    SDL_RenderTexture(app->renderer, app->texture_current, NULL, rect);

    return true;
}

static void rewind_bar_set_open_internal(App* app, bool enable, bool pop_last_state) {
    if (!app->rewind) {
        enable = false;
    }

    if (g_bar.enable != enable) {
        g_bar.enable = enable;

        if (g_bar.enable) {
            rewind_push_new_frame(app);
            g_bar.count = rewind_get_count(app->rewind);
            g_bar.cursor = g_bar.count - 1;
        } else {
            // rewind pushes the current frame to the buffer when the bar is opened.
            // if we are closing the bar as we did not load a state, then we want to
            // to remove that pushed state, otheriwse, by rapidly toggling the bar, we
            // would quikcly push many states to the buffer!
            if (pop_last_state) {
                rewind_remove_after(app->rewind, rewind_get_count(app->rewind) - 1);
            }

            // restore frame buffer.
            emulator_update_texture_pixels(app, app->pixel_buffer[app->pixel_buffer_index]);
        }
    }
}

void rewind_bar_set_open(App* app, bool enable) {
    rewind_bar_set_open_internal(app, enable, true);
}

bool rewind_bar_enabled(void) {
    return g_bar.enable;
}

void rewind_bar_button(App* app, enum RewindBarButton button) {
    if (!rewind_bar_enabled()) {
        return;
    }

    switch (button) {
        case RewindBarButton_OK:
            // load data at given index and remove all savestates after it.
            rewind_get(app->rewind, g_bar.cursor, app->rewind_buffer, app->rewind_buffer_size);
            rewind_remove_after(app->rewind, g_bar.cursor);

            // copy new frame to front and back buffer.
            SDL_memcpy(app->pixel_buffer[0], app->rewind_pixel_buffer, app->rewind_pixel_buffer_size);
            SDL_memcpy(app->pixel_buffer[1], app->rewind_pixel_buffer, app->rewind_pixel_buffer_size);

            // load savestate and disable the menu bar.
            SMS_loadstate(&app->sms, app->rewind_state_buffer, app->rewind_state_buffer_size, &app->rewind_state_config);
            rewind_bar_set_open_internal(app, false, false);
            break;

        case RewindBarButton_Back:
            rewind_bar_set_open_internal(app, false, true);
            break;

        case RewindBarButton_Left:
            rewind_bar_change_direction(-1);
            break;

        case RewindBarButton_Right:
            rewind_bar_change_direction(+1);
            break;
    }
}

void rewind_bar_render(App* app) {
    if (!rewind_bar_enabled()) {
        return;
    }

    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(app->renderer, &old_blend_mode);
    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect viewport;
    SDL_GetRenderViewport(app->renderer, &viewport);

    SDL_FRect bar;
    bar.x = 0;
    bar.y = viewport.h - BAR_HEIGHT;
    bar.w = viewport.w;
    bar.h = viewport.h - bar.y;

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0x47, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(app->renderer, &bar);

    const float centerx = (bar.x + bar.w) / 2;
    const float max_num_boxs = 4;
    const float padx = 10;
    const float pady_top = 15;
    const float pady_bottom = 5;
    const float boxw = bar.w / max_num_boxs;
    const float boxh = bar.h - pady_top - pady_bottom;

    SDL_FRect center_box;
    center_box.w = boxw;
    center_box.h = boxh;
    center_box.x = centerx - center_box.w / 2;
    center_box.y = bar.y + pady_top;

    // draw left
    SDL_FRect box = center_box;
    for (int i = g_bar.cursor - 1; i >= 0; i--) {
        box.x -= box.w + padx;
        if (box.x + box.w < bar.x) {
            break;
        }

        render_entry(app, i, &box, &bar);
    }

    // draw right
    box = center_box;
    for (int i = g_bar.cursor + 1; i < g_bar.count; i++) {
        box.x += box.w + padx;
        if (box.x > bar.x + bar.w) {
            break;
        }

        render_entry(app, i, &box, &bar);
    }

    // draw center
    render_entry(app, g_bar.cursor, &center_box, &bar);

    SDL_FRect rr;
    rr.x = 0;
    rr.y = 0;
    rr.w = viewport.w;
    rr.h = bar.y;
    SDL_RenderTexture(app->renderer, app->texture_current, NULL, &rr);

    SDL_SetRenderDrawBlendMode(app->renderer, old_blend_mode);
}
