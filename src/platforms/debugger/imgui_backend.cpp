// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "imgui_backend.hpp"
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <SDL.h>
#include <cstdio>
#include <unordered_map>

namespace {

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
std::unordered_map<Sint32, SDL_GameController*> controllers;
bool should_quit;

auto on_key_event(const SDL_KeyboardEvent& e) -> void
{
    const auto down = e.type == SDL_KEYDOWN;
    const auto ctrl = (e.keysym.mod & KMOD_CTRL) > 0;
    const auto shift = (e.keysym.mod & KMOD_SHIFT) > 0;

    if (ctrl)
    {
        if (down)
        {
            return;
        }

        if (shift)
        {
            switch (e.keysym.scancode)
            {
                // case SDL_SCANCODE_I:
                //     System::viewer_io ^= 1;
                //     break;

                // case SDL_SCANCODE_L:
                //     System::toggle_master_layer_enable();
                //     break;

                // case SDL_SCANCODE_A:
                //     System::gameboy_advance.bit_crushing ^= 1;
                //     break;

                default: break; // silence enum warning
            }
        }
        else
        {
            switch (e.keysym.scancode)
            {
                // case SDL_SCANCODE_P:
                //     System::emu_run ^= 1;
                //     break;

                // case SDL_SCANCODE_S:
                //     System::savestate(System::rom_path);
                //     break;

                // case SDL_SCANCODE_L:
                //     System::loadstate(System::rom_path);
                //     break;

                default: break; // silence enum warning
            }
        }

        return;
    }

    switch (e.keysym.scancode)
    {
        // case SDL_SCANCODE_X:      System::emu_set_button(gba::A, down);      break;
        // case SDL_SCANCODE_Z:      System::emu_set_button(gba::B, down);      break;
        // case SDL_SCANCODE_A:      System::emu_set_button(gba::L, down);      break;
        // case SDL_SCANCODE_S:      System::emu_set_button(gba::R, down);      break;
        // case SDL_SCANCODE_RETURN: System::emu_set_button(gba::START, down);  break;
        // case SDL_SCANCODE_SPACE:  System::emu_set_button(gba::SELECT, down); break;
        // case SDL_SCANCODE_UP:     System::emu_set_button(gba::UP, down);     break;
        // case SDL_SCANCODE_DOWN:   System::emu_set_button(gba::DOWN, down);   break;
        // case SDL_SCANCODE_LEFT:   System::emu_set_button(gba::LEFT, down);   break;
        // case SDL_SCANCODE_RIGHT:  System::emu_set_button(gba::RIGHT, down);  break;

    #ifndef EMSCRIPTEN
        case SDL_SCANCODE_ESCAPE:
            should_quit = true;
            break;
    #endif // EMSCRIPTEN

        default: break; // silence enum warning
    }
}

auto on_display_event(const SDL_DisplayEvent& e) -> void
{

}

auto on_window_event(const SDL_WindowEvent& e) -> void
{
    switch (e.event)
    {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_HIDDEN:
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_MOVED:
        case SDL_WINDOWEVENT_RESIZED:
            break;

        case SDL_WINDOWEVENT_SIZE_CHANGED:
            // System::resize_emu_screen();
            break;

        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_LEAVE:
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        case SDL_WINDOWEVENT_FOCUS_LOST:
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_WINDOWEVENT_TAKE_FOCUS:
        case SDL_WINDOWEVENT_HIT_TEST:
            break;
    }
}

auto on_dropfile_event(SDL_DropEvent& e) -> void
{
    if (e.file != nullptr)
    {
        // System::loadrom(e.file);
        SDL_free(e.file);
    }
}

auto on_controlleraxis_event(const SDL_ControllerAxisEvent& e) -> void
{
    // sdl recommends deadzone of 8000
    constexpr auto DEADZONE = 8000;
    constexpr auto LEFT     = -DEADZONE;
    constexpr auto RIGHT    = +DEADZONE;
    constexpr auto UP       = -DEADZONE;
    constexpr auto DOWN     = +DEADZONE;

    switch (e.axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX:
        case SDL_CONTROLLER_AXIS_RIGHTX:
            if (e.value < LEFT)
            {
                // System::emu_set_button(gba::LEFT, true);
            }
            else if (e.value > RIGHT)
            {
                // System::emu_set_button(gba::RIGHT, true);
            }
            else
            {
                // System::emu_set_button(gba::LEFT, false);
                // System::emu_set_button(gba::RIGHT, false);
            }
            break;

        case SDL_CONTROLLER_AXIS_LEFTY:
        case SDL_CONTROLLER_AXIS_RIGHTY:
            if (e.value < UP)
            {
                // System::emu_set_button(gba::UP, true);
            }
            else if (e.value > DOWN)
            {
                // System::emu_set_button(gba::DOWN, true);
            }
            else
            {
            {
                // System::emu_set_button(gba::UP, false);
                // System::emu_set_button(gba::DOWN, false);
            }
            }
            break;

        // don't handle yet
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            return;

        default: return; // silence enum warning
    }
}

auto on_controllerbutton_event(const SDL_ControllerButtonEvent& e) -> void
{
    const auto down = e.type == SDL_CONTROLLERBUTTONDOWN;

    switch (e.button)
    {
        // case SDL_CONTROLLER_BUTTON_A: System::emu_set_button(gba::A, down); break;
        // case SDL_CONTROLLER_BUTTON_B: System::emu_set_button(gba::B, down); break;
        // case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: System::emu_set_button(gba::L, down); break;
        // case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: System::emu_set_button(gba::R, down); break;
        // case SDL_CONTROLLER_BUTTON_START: System::emu_set_button(gba::START, down); break;
        // case SDL_CONTROLLER_BUTTON_GUIDE: System::emu_set_button(gba::SELECT, down); break;
        // case SDL_CONTROLLER_BUTTON_DPAD_UP: System::emu_set_button(gba::UP, down); break;
        // case SDL_CONTROLLER_BUTTON_DPAD_DOWN: System::emu_set_button(gba::DOWN, down); break;
        // case SDL_CONTROLLER_BUTTON_DPAD_LEFT: System::emu_set_button(gba::LEFT, down); break;
        // case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: System::emu_set_button(gba::RIGHT, down); break;

        default: break; // silence enum warning
    }
}

auto on_controllerdevice_event(const SDL_ControllerDeviceEvent& e) -> void
{
    switch (e.type)
    {
        case SDL_CONTROLLERDEVICEADDED: {
            const auto itr = controllers.find(e.which);
            if (itr == controllers.end())
            {
                auto controller = SDL_GameControllerOpen(e.which);
                if (controller != nullptr)
                {
                    std::printf("[CONTROLLER] opened: %s\n", SDL_GameControllerNameForIndex(e.which));
                }
                else
                {
                    std::printf("[CONTROLLER] failed to open: %s error: %s\n", SDL_GameControllerNameForIndex(e.which), SDL_GetError());
                }
            }
            else
            {
               std::printf("[CONTROLLER] already added, ignoring: %s\n", SDL_GameControllerNameForIndex(e.which));
            }
        } break;

        case SDL_CONTROLLERDEVICEREMOVED: {
            const auto itr = controllers.find(e.which);
            if (itr != controllers.end())
            {
                std::printf("[CONTROLLER] removed controller\n");

                // have to manually close to free struct
                SDL_GameControllerClose(itr->second);
                controllers.erase(itr);
            }
        } break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            std::printf("mapping updated for: %s\n", SDL_GameControllerNameForIndex(e.which));
            break;
    }
}

} // namespace

namespace backend {

auto init() -> bool
{
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
    {
        return false;
    }

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, 100, 100);

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    should_quit = false;

    return true;
}

auto quit() -> void
{
    should_quit = true;

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    if (texture)
    {
        SDL_DestroyTexture(texture);
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
}

auto begin_frame() -> void
{
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
}

auto end_frame() -> void
{
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}

auto quit_requested() -> bool
{
    return should_quit;
}

auto process_events() -> void
{
    SDL_Event e{};

    while (SDL_PollEvent(&e) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&e);

        switch (e.type)
        {
            case SDL_QUIT:
                should_quit = true;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key_event(e.key);
                break;

            case SDL_DISPLAYEVENT:
                on_display_event(e.display);
                break;

            case SDL_WINDOWEVENT:
                on_window_event(e.window);
                break;

            case SDL_CONTROLLERAXISMOTION:
                on_controlleraxis_event(e.caxis);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                on_controllerbutton_event(e.cbutton);
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                on_controllerdevice_event(e.cdevice);
                break;

            case SDL_DROPFILE:
                on_dropfile_event(e.drop);
                break;

            case SDL_DROPTEXT:
            case SDL_DROPBEGIN:
            case SDL_DROPCOMPLETE:

            case SDL_APP_TERMINATING:
            case SDL_APP_LOWMEMORY:
            case SDL_APP_WILLENTERBACKGROUND:
            case SDL_APP_DIDENTERBACKGROUND:
            case SDL_APP_WILLENTERFOREGROUND:
            case SDL_APP_DIDENTERFOREGROUND:
            case SDL_LOCALECHANGED:
            case SDL_SYSWMEVENT:
            case SDL_TEXTEDITING:
            case SDL_TEXTINPUT:
            case SDL_KEYMAPCHANGED:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERTOUCHPADDOWN:
            case SDL_CONTROLLERTOUCHPADMOTION:
            case SDL_CONTROLLERTOUCHPADUP:
            case SDL_CONTROLLERSENSORUPDATE:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
            case SDL_DOLLARGESTURE:
            case SDL_DOLLARRECORD:
            case SDL_MULTIGESTURE:
            case SDL_CLIPBOARDUPDATE:
            case SDL_AUDIODEVICEADDED:
            case SDL_AUDIODEVICEREMOVED:
            case SDL_SENSORUPDATE:
            case SDL_RENDER_TARGETS_RESET:
            case SDL_RENDER_DEVICE_RESET:
            case SDL_USEREVENT:
                break;

            default: break; // silence enum warning
        }
    }
}

} // namespace backend
