#pragma once

#include <stddef.h>
#include "app.h"

enum RewindBarButton {
    RewindBarButton_OK,
    RewindBarButton_Back,
    RewindBarButton_Left,
    RewindBarButton_Right,
};

void rewind_bar_set_open(App* app, bool enable);
bool rewind_bar_enabled(void);
void rewind_bar_button(App* app, enum RewindBarButton button);
void rewind_bar_render(App* app);
