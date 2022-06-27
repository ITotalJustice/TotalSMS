// Copyright 2022 TotalJustice.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace backend {

auto init() -> bool;
auto quit() -> void;

auto begin_frame() -> void;
auto end_frame() -> void;

auto quit_requested() -> bool;
auto process_events() -> void;

auto get_texture() -> void*;

} // namespace backend
