#ifndef ZED_EDITOR_H
#define ZED_EDITOR_H

#include <stdint.h>

#include "buffer.h"

typedef struct {
    uint64_t running;
    uint32_t buffersLength;
    uint32_t buffersSize;
    zed_Buffer **buffers;
    zed_Buffer *currentBuffer;
} zed_Editor;

zed_Editor *zed_EditorCreate();
zed_error_t zed_EditorOpen(zed_Editor *, char *);
void zed_EditorDelete(zed_Editor *);

#endif
