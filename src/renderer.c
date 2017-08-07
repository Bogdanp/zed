#include <stdint.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"
#include "renderer.h"

void zed_ClearScreen() {
    fprintf(stdout, "[2J");
}

void zed_MoveCursor(uint64_t line, uint64_t column) {
    char sbuf[100];
    sprintf(sbuf, "[%lld;%lldH", line, column);
    fprintf(stdout, "%s", sbuf);
}

/**
 * zed_SetTerminalMode sets the terminal mode. While in raw mode,
 * terminals don't attempt to interpret escape sequences.
 */
void zed_SetTerminalMode(zed_TerminalMode mode) {
    static struct termios oldTermios, rawTermios;

    switch (mode) {
    case ZED_COOKED_TERMINAL:
        tcgetattr(STDIN_FILENO, &oldTermios);
        rawTermios = oldTermios;
        rawTermios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        rawTermios.c_oflag &= ~(OPOST);
        rawTermios.c_cflag |= (CS8);
        rawTermios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        rawTermios.c_cc[VMIN] = 5; rawTermios.c_cc[VTIME] = 8;
        rawTermios.c_cc[VMIN] = 0; rawTermios.c_cc[VTIME] = 0;
        rawTermios.c_cc[VMIN] = 2; rawTermios.c_cc[VTIME] = 0;
        rawTermios.c_cc[VMIN] = 0; rawTermios.c_cc[VTIME] = 8;
        tcsetattr(STDIN_FILENO, TCSANOW, &rawTermios);
        break;
    case ZED_RAW_TERMINAL:
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
        break;
    }
}

/**
 * zed_ResetTerminalMode sets the terminal mode back to normal.
 */
void zed_ResetTerminalMode() {
    zed_ClearScreen();
    zed_MoveCursor(1, 1);
    zed_SetTerminalMode(ZED_COOKED_TERMINAL);
}

/**
 * zed_HandleInput blocks and handles input keystrokes.
 */
void zed_HandleInput(zed_Editor *editor) {
    zed_Buffer *buffer = editor->currentBuffer;

    char c;
    while (0 == read(STDIN_FILENO, &c, 1));

    switch (c) {
    case '':
        editor->running = 0;
        break;

    case '':
        zed_BufferMoveCursor(buffer, ZED_CURSOR_UP);
        break;

    case '':
        zed_BufferMoveCursor(buffer, ZED_CURSOR_DOWN);
        break;

    case '':
        zed_BufferMoveCursor(buffer, ZED_CURSOR_LEFT);
        break;

    case '':
        zed_BufferMoveCursor(buffer, ZED_CURSOR_RIGHT);
        break;

    case '\r':
        c = '\n';

    default:
        zed_BufferInsert(buffer, buffer->cursor, 1, (int8_t*)&c);
        zed_BufferMoveCursor(buffer, ZED_CURSOR_RIGHT);
    }
}

/**
 * zed_Render renders the editor to the terminal.
 */
void zed_Render(zed_Editor *editor) {
    zed_ClearScreen();
    zed_MoveCursor(1, 1);

    zed_Buffer *buffer = editor->currentBuffer;
    zed_Piece *piece = buffer->pieces;
    while (NULL != piece) {
        for (uint64_t i = piece->start; i < piece->end; i++) {
            int8_t c = buffer->arena[i];

            if (c == '\n') {
                printf("\r\n");
            } else {
                putchar(c);
            }
        }

        piece = piece->next;
    }


    uint64_t line, column;
    zed_BufferComputePosition(buffer, &line, &column);
    zed_MoveCursor(50, 1);
    printf("Cursor: %lld (%lld, %lld)", buffer->cursor, line + 1, column + 1);
    zed_MoveCursor(line + 1, column + 1);

    fflush(stdout);
}
