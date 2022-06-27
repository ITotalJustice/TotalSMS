// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "psg_debugger.hpp"
#include <SDL_audio.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <imgui.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <SDL.h>

namespace {

constexpr auto SAMPLE_ARRAY_SIZE = SMS_CYCLES_PER_FRAME / (SMS_CPU_CLOCK / 48000);
float tone0[900];
float tone1[900];
float tone2[900];
float noise[900];

bool enable_tone0{true};
bool enable_tone1{true};
bool enable_tone2{true};
bool enable_noise{true};

std::size_t counter = 0;
SDL_AudioDeviceID audio_device_id;

auto sms_apu_callback2(void* user, SMS_ApuCallbackData* data) -> void
{
    if (counter < SAMPLE_ARRAY_SIZE)
    {
        tone0[counter] = data->tone0[0];
        tone1[counter] = data->tone1[0];
        tone2[counter] = data->tone2[0];
        noise[counter] = data->noise[0];
        counter++;
    }
}

auto mixer() -> void
{
    std::uint8_t samples[900]{};

    for (std::size_t i = 0; i < counter; i++)
    {
        if (enable_tone0) { samples[i] += tone0[i]; }
        if (enable_tone1) { samples[i] += tone1[i]; }
        if (enable_tone2) { samples[i] += tone2[i]; }
        if (enable_noise) { samples[i] += noise[i]; }

        samples[i] *= 2;
        samples[i] += 128;
    }

    // if (counter && counter < SAMPLE_ARRAY_SIZE)
    // {
    //     for (std::size_t i = counter; i < SAMPLE_ARRAY_SIZE; i++)
    //     {
    //         samples[i] = samples[counter - 1];
    //     }
    // }

    // auto max = std::max((int)counter, SAMPLE_ARRAY_SIZE);
    SDL_QueueAudio(audio_device_id, samples, counter);
}

} // namespace

namespace debugger::psg {

auto show(SMS_Core& master_system, bool* p_open) -> void
{
    static bool once = false;
    if (!once)
    {
        SDL_AudioSpec spec{};
        spec.freq = 48000;
        spec.channels = 1;
        spec.samples = 1024*4;
        spec.format = AUDIO_U8;

        audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
        SDL_PauseAudioDevice(audio_device_id, 0);

        once = true;
    }

    if (!*p_open)
    {
        return;
    }

    mixer();

    if (ImGui::Begin("psg", p_open))
    {
        SMS_set_apu_callback(&master_system, sms_apu_callback2, 48000);

        float samples[900];

        for (std::size_t i = 0; i < counter; i++)
        {
            samples[i] = tone0[i] + tone1[i] + tone2[i] + noise[i];
        }

        ImGui::PlotLines("##tone0", tone0, counter, 0, nullptr, 0, 15, ImVec2(0, 100));
        ImGui::SameLine();
        ImGui::Checkbox("tone0", &enable_tone0);

        ImGui::PlotLines("##tone1", tone1, counter, 0, nullptr, 0, 15, ImVec2(0, 100));
        ImGui::SameLine();
        ImGui::Checkbox("tone1", &enable_tone1);

        ImGui::PlotLines("##tone2", tone2, counter, 0, nullptr, 0, 15, ImVec2(0, 100));
        ImGui::SameLine();
        ImGui::Checkbox("tone2", &enable_tone2);

        ImGui::PlotLines("##noise", noise, counter, 0, nullptr, 0, 15, ImVec2(0, 100));
        ImGui::SameLine();
        ImGui::Checkbox("noise", &enable_noise);

        ImGui::PlotLines("samples", samples, counter, 0, nullptr, 0, 60, ImVec2(0, 100));
    }
    ImGui::End();

    counter = 0;
}

} // namespace debugger::psg
