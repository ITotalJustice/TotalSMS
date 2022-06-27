// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "z80_debugger.hpp"
#include <imgui.h>

namespace debugger::z80 {

auto show(SMS_Core& master_system, bool* p_open) -> void
{
    if (!*p_open)
    {
        return;
    }

    if (ImGui::Begin("z80", p_open))
    {
    }
    ImGui::End();
}

} // namespace debugger::z80
