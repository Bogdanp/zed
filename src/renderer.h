#ifndef ZED_RENDERER_H
#define ZED_RENDERER_H

#include <stdint.h>

#include "editor.h"

typedef enum {
    ZED_COOKED_TERMINAL,
    ZED_RAW_TERMINAL,
} zed_TerminalMode;

void zed_SetTerminalMode(zed_TerminalMode);
void zed_ResetTerminalMode();
void zed_Render(zed_Editor *);
void zed_HandleInput(zed_Editor *);

#endif
