#pragma once

#include "text_popup.h"

#include <sms.h>
#include <SDL3/SDL.h>
#include <stddef.h>
#include <stdbool.h>

enum VdpRatioType {
    VdpRatioType_NONE,
    VdpRatioType_AUTO,
    VdpRatioType_NTSC,
    VdpRatioType_PAL,
    VdpRatioType_GG,
};

struct Runahead {
    uint8_t** states;
    unsigned count;
    unsigned frames;
    size_t state_size;
    bool lock_input; // if true, locks input.
    bool lazy; // if true, uses more optimised version.
};

struct Input {
    uint16_t button;
};

struct Gamepad {
    SDL_JoystickID id;
    SDL_Gamepad* pad;
    bool button[SDL_GAMEPAD_BUTTON_COUNT];
    bool axis[SDL_GAMEPAD_AXIS_COUNT];
};

typedef struct {
    // sdl stuff
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture_current;
    SDL_Texture* texture_previous;
    SDL_AudioStream* audio_stream;
    const SDL_PixelFormatDetails* pixel_format_details;
    SDL_PixelFormat pixel_format;
    bool keys[SDL_SCANCODE_COUNT];

    // todo: support multiple controllers.
    struct Gamepad gamepad;

    // vars
    struct SMS_Core sms;
    void* pixel_buffer;
    struct Runahead runahead;
    struct Input inputs[2]; // [0] current [1 previous]
    uint32_t overscan_colour;
    struct TextPopup text_popup;

    // allocated sample buffer for audio callbacks.
    int16_t* sample_data;

    // config
    enum VdpRatioType ratio;
    int sms_scale;
    int gg_scale;
    int window_w;
    int window_h;
    // blends previous frame with new frame.
    bool frame_blending;
    // uses overscan colour to fill the screen.
    bool overscan_fill;

    bool paused;
    bool focus;
    bool quit;
} App;
