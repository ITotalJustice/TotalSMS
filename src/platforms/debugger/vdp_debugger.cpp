// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "vdp_debugger.hpp"
#include <imgui.h>

namespace debugger::vdp {

auto show(SMS_Core& master_system, bool* p_open) -> void
{
    if (!*p_open)
    {
        return;
    }

    if (ImGui::Begin("vdp", p_open))
    {
    }
    ImGui::End();
}

} // namespace debugger::vdp
