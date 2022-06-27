// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <sms.h>

namespace debugger::mapper {

auto show(SMS_Core& master_system, bool* p_open) -> void;

} // namespace debugger::mapper
