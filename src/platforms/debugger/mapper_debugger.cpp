// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#include "mapper_debugger.hpp"
#include <imgui.h>

namespace debugger::mapper {

auto show(SMS_Core& master_system, bool* p_open) -> void
{
    if (!*p_open)
    {
        return;
    }

    if (ImGui::Begin("mapper", p_open))
    {
    }
    ImGui::End();
}

} // namespace debugger::mapper
