// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <sms.h>

namespace debugger::z80 {

auto show(SMS_Core& master_system, bool* p_open) -> void;

} // namespace debugger::z80
