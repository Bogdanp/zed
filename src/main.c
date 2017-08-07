#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "renderer.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: zed FILENAME [FILENAME [FILENAME ...]]\n");
        return 1;
    }

    zed_Editor *editor = zed_EditorCreate();
    for (int i = 1; i < argc; i++) {
        zed_EditorOpen(editor, argv[i]);
    }

    zed_SetTerminalMode(ZED_RAW_TERMINAL);
    atexit(zed_ResetTerminalMode);

    while (editor->running == 1) {
        zed_Render(editor);
        zed_HandleInput(editor);
    }

    zed_EditorDelete(editor);
    return 0;
}
