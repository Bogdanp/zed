#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "error.h"

/**
 * zed_EditorCreate creates a new zed_Editor record.
 */
zed_Editor *zed_EditorCreate() {
    zed_Editor *editor = malloc(sizeof(zed_Editor));
    if (NULL == editor) {
        return NULL;
    }

    editor->running = 1;
    editor->buffersLength = 0;
    editor->buffersSize = 1024;
    editor->buffers = malloc(sizeof(zed_Buffer*) * editor->buffersSize);
    if (NULL == editor->buffers) {
        free(editor);
        return NULL;
    }

    editor->currentBuffer = NULL;
    return editor;
}

zed_error_t zed_EditorBuffersAppend(zed_Editor *editor, zed_Buffer *buffer) {
    if (editor->buffersLength == editor->buffersSize - 1) {
        editor->buffersSize *= 2;
        editor->buffers = (zed_Buffer**)realloc(editor->buffers, sizeof(zed_Buffer*) * editor->buffersSize);
        if (NULL == editor->buffers) {
            return ZED_ALLOC_FAILED;
        }
    }

    editor->buffers[editor->buffersLength++] = buffer;
    return ZED_OK;
}

/**
 * zed_EditorOpen opens a file in a buffer.  If the file is already
 * open, its associated buffer is made current.
 *
 * @param editor The Editor record.
 * @param filename The name of the file to open.
 */
zed_error_t zed_EditorOpen(zed_Editor *editor, char *filename) {
    zed_Buffer *buffer;
    for (uint32_t i = 0; i < editor->buffersLength; i++) {
        buffer = editor->buffers[i];
        if (0 == memcmp(buffer->filename, filename, strlen(filename))) {
            editor->currentBuffer = buffer;
            return ZED_OK;
        }
    }

    buffer = zed_BufferCreate(filename);
    zed_BufferLoad(buffer, filename);
    editor->currentBuffer = buffer;
    return zed_EditorBuffersAppend(editor, buffer);
}

/**
 * zed_EditorDelete frees a zed_Editor.
 */
void zed_EditorDelete(zed_Editor *editor) {
    for (uint32_t i = 0; i < editor->buffersLength; i++) {
        zed_BufferDelete(editor->buffers[i]);
    }

    free(editor->buffers);
    free(editor);
}
