#ifndef ZED_BUFFER_H
#define ZED_BUFFER_H

#include <stdint.h>

#include "error.h"

typedef struct Piece {
    uint64_t start;
    uint64_t end;

    struct Piece *next;
} zed_Piece;

zed_Piece *zed_PieceCreate(uint64_t, uint64_t);
uint64_t zed_PieceLength(zed_Piece *);
void zed_PieceDelete(zed_Piece *);


typedef enum {
    ZED_CURSOR_UP,
    ZED_CURSOR_DOWN,
    ZED_CURSOR_LEFT,
    ZED_CURSOR_RIGHT,
} zed_CursorDirection;

typedef struct {
    zed_Piece *prevPiece;
    zed_Piece *currPiece;
    uint64_t cursor;
} zed_HistoryItem;

typedef struct {
    char *filename;

    uint64_t cursor;
    zed_Piece *pieces;

    uint64_t arenaLength;
    uint64_t arenaSize;
    int8_t *arena;

    uint64_t historyLength;
    uint64_t historySize;
    zed_HistoryItem **history;
} zed_Buffer;

zed_Buffer *zed_BufferCreate(char *);
zed_error_t zed_BufferLoad(zed_Buffer *, char *);
zed_error_t zed_BufferInsert(zed_Buffer *, uint64_t, uint64_t, int8_t *);
zed_error_t zed_BufferMoveCursor(zed_Buffer *, zed_CursorDirection);
zed_error_t zed_BufferComputePosition(zed_Buffer *, uint64_t *, uint64_t *);
uint64_t zed_BufferLength(zed_Buffer *);
void zed_BufferDump(zed_Buffer *, int8_t *);
void zed_BufferDelete(zed_Buffer *);

#endif
