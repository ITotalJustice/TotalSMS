#pragma once

#include "rewind.h"

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

// data shared between the audio thread should be copied here
// changes should be made whilst SDL_LockAudioStream is in affect.
struct AudioSharedData {
    int speed_index;
};

// see above.
struct TimerSharedData {
    // to access data in this struct, lock the mutex before hand.
    SDL_Mutex* mutex;
    // incremented every vblank.
    int vblank_counter;
    // incremented every gui itteration.
    int gui_counter;

    // stored value when the timer fires.
    int vblank_fps;
    int gui_fps;
    // set to true when new data is pending.
    bool pending;
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
    struct AudioSharedData audio_shared_data;

    // runs every 1s, used for calculating fps.
    SDL_TimerID timer;
    struct TimerSharedData timer_shared_data;

    // vars
    struct SMS_Core sms;
    void* pixel_buffer[2];
    size_t pixel_buffer_size;
    bool pixel_buffer_index;
    bool pending_frame;

    struct Runahead runahead;
    struct Input inputs[2]; // [0] current [1 previous]
    uint32_t overscan_colour;

    Rewind* rewind;
    void* rewind_buffer;
    size_t rewind_buffer_size;
    // increases
    int vblank_fps_counter;
    // counts down every vblank.
    size_t rewind_counter;
    // set to true when counter hits 0.
    bool rewind_should_push;

    // these point to the above buffer, do not free!
    void* rewind_pixel_buffer;
    size_t rewind_pixel_buffer_size;
    void* rewind_state_buffer;
    size_t rewind_state_buffer_size;
    struct SMS_StateConfig rewind_state_config;

    // allocated sample buffer for audio callbacks.
    int16_t* sample_data;

    // config
    SDL_RendererLogicalPresentation stretch;
    enum VdpRatioType ratio;
    int scale;
    // the actual window size.
    int window_w;
    int window_h;
    // sizes of the emulated screen.
    int screen_w;
    int screen_h;
    // blends previous frame with new frame.
    bool frame_blending;
    // uses overscan colour to fill the screen.
    bool overscan_fill;

    // how often to save a new frame.
    int rewind_keyframe_interval;
    // how many seconds of frames to store.
    int rewind_num_seconds;

    int speed_index;
    bool paused;
    bool focus;
    bool quit;
} App;

bool rewind_push_new_frame(App* app);
void emulator_update_texture_pixels(App* app, const void* pixel_buffer);
