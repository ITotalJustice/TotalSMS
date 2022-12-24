// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "imgui_backend.hpp"
#include "io_debugger.hpp"
#include "mapper_debugger.hpp"
#include "psg_debugger.hpp"
#include "sms_types.h"
#include "vdp_debugger.hpp"
#include "z80_debugger.hpp"
#include "trim_font.hpp"

#include <imgui.h>
#include <sms.h>
#include <mgb.h>

#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <stop_token>
#include <thread>
#include <condition_variable>

namespace {

SMS_Core master_system;

uint32_t pixels[SMS_SCREEN_HEIGHT][SMS_SCREEN_WIDTH];

std::mutex emulator_thread_mutex;
std::condition_variable emulator_thread_cond_var;
std::jthread emulator_thread;

std::string rom_path;

bool show_mapper_debugger{true};
bool show_psg_debugger{true};
bool show_vdp_debugger{true};
bool show_z80_debugger{true};

auto emulator_thread_func(const std::stop_token& stop_token) -> void
{
    for (;;)
    {
        std::unique_lock lock{emulator_thread_mutex};
        emulator_thread_cond_var.wait(lock);

        if (stop_token.stop_requested())
        {
            std::printf("woke up thread\n");
            return;
        }

        if (mgb_has_rom())
        {
            // todo: process inputs here
            // SMS_run(&master_system, SMS_CYCLES_PER_FRAME);
        }
    }
}

auto loadrom(const std::string& path) -> bool
{
    std::unique_lock lock{emulator_thread_mutex};

    if (!mgb_load_rom_file(path.c_str()))
    {
        return false;
    }

    rom_path = path;

    return true;
}

auto process_events() -> void
{
    backend::process_events();
}

auto run() -> void
{
    if (mgb_has_rom())
    {
        SMS_run(&master_system, SMS_CYCLES_PER_FRAME);
    }
}

auto render() -> void
{
    backend::begin_frame();
    ImGui::NewFrame();
    {
        static bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        debugger::mapper::show(master_system, &show_mapper_debugger);
        debugger::psg::show(master_system, &show_psg_debugger);
        debugger::vdp::show(master_system, &show_vdp_debugger);
        debugger::z80::show(master_system, &show_z80_debugger);
    }
    ImGui::Render();
    backend::end_frame();
}

auto sms_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b) -> uint32_t
{
    return 0;
}

auto sms_apu_callback(void* user, SMS_ApuCallbackData* data) -> void
{
}

auto sms_vblank_callback(void* user) -> void
{
}

} // namespace

namespace debugger {

auto init() -> bool
{
    emulator_thread = std::jthread(emulator_thread_func);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    io.Fonts->AddFontFromMemoryCompressedTTF(trim_font_compressed_data, trim_font_compressed_size, 20);

    return backend::init();
}

auto loop() -> void
{
    while (!backend::quit_requested())
    {
        // wake up emulator thread
        emulator_thread_cond_var.notify_all();

        process_events();
        run();
        render();
    }
}

auto quit() -> void
{
    if (mgb_has_rom())
    {
        mgb_save_save_file(nullptr);
    }

    if (emulator_thread.joinable())
    {
        emulator_thread_cond_var.notify_all();
        emulator_thread.request_stop();
        emulator_thread.join();
    }

    backend::quit();
    ImGui::DestroyContext();
}

} // namespace debugger

auto main(int argc, char** argv) -> int
{
    SMS_init(&master_system);
    mgb_init(&master_system);

    SMS_set_colour_callback(&master_system, sms_colour_callback);
    SMS_set_pixels(&master_system, pixels, SMS_SCREEN_WIDTH, 32);
    SMS_set_apu_callback(&master_system, sms_apu_callback, 48000);
    SMS_set_vblank_callback(&master_system, sms_vblank_callback);

    if (argc >= 2)
    {
        if (!loadrom(argv[1]))
        {
            return -1;
        }
    }

    assert(master_system.psg.channel_enable[0][0]);
    assert(master_system.psg.channel_enable[1][0]);
    assert(master_system.psg.channel_enable[2][0]);
    assert(master_system.psg.channel_enable[3][0]);

    debugger::init();
    debugger::loop();
    debugger::quit();

    return 0;
}
