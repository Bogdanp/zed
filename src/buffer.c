#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/**
 * zed_PieceCreate creates a new zed_Piece record.
 *
 * @param start The start position of this piece in the arena.
 * @param end The end position of this piece in the arena.
 */
zed_Piece *zed_PieceCreate(uint64_t start, uint64_t end) {
    zed_Piece *piece = malloc(sizeof(zed_Piece));
    if (NULL == piece) {
        return NULL;
    }

    piece->start = start;
    piece->end = end;
    piece->next = NULL;
    return piece;
}

/**
 * zed_PieceLength returns the total length of a zed_Piece.
 */
inline uint64_t zed_PieceLength(zed_Piece *piece) {
    return piece->end - piece->start;
}

/**
 * zed_PieceDelete frees a zed_Piece record.
 */
inline void zed_PieceDelete(zed_Piece *piece) {
    zed_Piece *next = piece->next;

    free(piece);
    while (NULL != next) {
        piece = next;
        next = piece->next;
        free(piece);
    }
}

void zed_PiecePrint(zed_Piece *piece) {
    printf("Piece(%lld, %lld, %p)", piece->start, piece->end, piece->next);
}


/**
 * zed_BufferCreate creates a new zed_Buffer record.
 *
 * @param filename
 */
zed_Buffer *zed_BufferCreate(char *filename) {
    zed_Buffer *buffer = malloc(sizeof(zed_Buffer));
    if (NULL == buffer) {
        return NULL;
    }

    buffer->filename = filename;
    buffer->cursor = 0;
    buffer->pieces = NULL;

    buffer->arenaLength = 0;
    buffer->arenaSize = 1024;
    buffer->arena = malloc(sizeof(int8_t) * buffer->arenaSize);
    if (NULL == buffer->arena) {
        free(buffer);
        return NULL;
    }

    buffer->historyLength = 0;
    buffer->historySize = 1024;
    buffer->history = malloc(sizeof(zed_HistoryItem) * buffer->historySize);
    if (NULL == buffer->history) {
        free(buffer->arena);
        free(buffer);
        return NULL;
    }

    return buffer;
}

zed_error_t zed_BufferArenaAppend(zed_Buffer *buffer, uint64_t size, int8_t *text) {
    // TODO: Potentially bad if appending a large amount of stuff.
    while (buffer->arenaSize - buffer->arenaLength < size) {
        buffer->arenaSize *= 2;
        buffer->arena = (int8_t *)realloc(buffer->arena, sizeof(int8_t) * buffer->arenaSize);
        if (NULL == buffer->arena) {
            return ZED_ALLOC_FAILED;
        }
    }

    memcpy(buffer->arena + buffer->arenaLength, text, size);
    buffer->arenaLength += size;
    return ZED_OK;
}

zed_error_t zed_BufferHistoryAppend(zed_Buffer *buffer, zed_Piece *prevPiece, zed_Piece *currPiece) {
    zed_HistoryItem *item = malloc(sizeof(zed_HistoryItem));
    if (NULL == item) {
        return ZED_ALLOC_FAILED;
    }

    item->prevPiece = prevPiece;
    item->currPiece = currPiece;
    item->cursor = buffer->cursor;

    if (buffer->historyLength == buffer->historySize - 1) {
        buffer->historySize *= 2;
        buffer->history = (zed_HistoryItem**)realloc(buffer->history, buffer->historySize);
    }

    buffer->history[buffer->historyLength++] = item;
    return ZED_OK;
}

/**
 * zed_BufferLoad loads a buffer from a file.
 */
zed_error_t zed_BufferLoad(zed_Buffer *buffer, char *filename) {
    FILE *stream = fopen(filename, "r");
    if (NULL == stream) {
        return ZED_FILE_NOT_FOUND;
    }

    // Truncate the buffer
    if (NULL != buffer->pieces) {
        zed_BufferHistoryAppend(buffer, NULL, buffer->pieces);
        buffer->pieces = NULL;
    }

    // Then read the whole file into memory
    fseek(stream, 0, SEEK_END);
    size_t fsize = ftell(stream);
    int8_t *buff = malloc(sizeof(int8_t) * fsize);
    fseek(stream, 0, SEEK_SET);
    fread(buff, sizeof(int8_t), fsize, stream);
    fclose(stream);

    // And fill the buffer
    zed_error_t res = zed_BufferInsert(buffer, 0, fsize, buff);
    free(buff);

    return res;
}

/**
 * zed_BufferInsert inserts some text into a buffer.
 *
 * @param buffer A pointer to the zed_Buffer.
 * @param offset The offset after which to insert the text.
 * @param size The number of characters being inserted.
 * @param text A pointer to the text being inserted.
 */
zed_error_t zed_BufferInsert(zed_Buffer *buffer, uint64_t offset, uint64_t size, int8_t *text) {
    zed_error_t err = zed_BufferArenaAppend(buffer, size, text);
    if (0 != err) {
        return err;
    }

    if (NULL == buffer->pieces) {
        buffer->pieces = zed_PieceCreate(0, size);
        return zed_BufferHistoryAppend(buffer, NULL, buffer->pieces);
    }

    zed_Piece *prevPiece = buffer->pieces;
    zed_Piece *piece = buffer->pieces;
    uint64_t pieceStartOffset = 0;
    uint64_t pieceEndOffset = 0;
    uint64_t innerOffset;
    while (NULL != piece) {
        pieceStartOffset = pieceEndOffset;
        pieceEndOffset = pieceEndOffset + zed_PieceLength(piece);
        if (pieceStartOffset <= offset && offset < pieceEndOffset) {
            break;
        }

        prevPiece = piece;
        piece = piece->next;
    }

    // We were told to insert text after the final piece so we just
    // append a new piece.
    if (NULL == piece) {
        piece = zed_PieceCreate(buffer->arenaLength - size, buffer->arenaLength);
        prevPiece->next = piece;
        return zed_BufferHistoryAppend(buffer, prevPiece, piece);

    // We were told to insert text right before a piece.
    } else if (pieceStartOffset == offset) {
        zed_Piece *nPiece = zed_PieceCreate(buffer->arenaLength - size, buffer->arenaLength);

        nPiece->next = piece;

        if (prevPiece == piece) {
            buffer->pieces = nPiece;
            return zed_BufferHistoryAppend(buffer, NULL, nPiece);
        } else {
            prevPiece->next = nPiece;
            return zed_BufferHistoryAppend(buffer, prevPiece, nPiece);
        }

    // We were told to insert text in the middle of a piece so we have
    // to split it in two and insert another piece in the middle.
    } else {
        innerOffset = offset - pieceStartOffset;

        zed_Piece *lPiece = zed_PieceCreate(piece->start, piece->start + innerOffset);
        zed_Piece *mPiece = zed_PieceCreate(buffer->arenaLength - size, buffer->arenaLength);
        zed_Piece *rPiece = zed_PieceCreate(piece->start + innerOffset, piece->end);

        lPiece->next = mPiece;
        mPiece->next = rPiece;
        rPiece->next = piece->next;

        if (prevPiece == piece) {
            buffer->pieces = lPiece;
            return zed_BufferHistoryAppend(buffer, NULL, lPiece);
        } else {
            prevPiece->next = lPiece;
            return zed_BufferHistoryAppend(buffer, prevPiece, lPiece);
        }
    }
}

void zed_BufferGoTo(zed_Buffer *buffer, uint32_t targetLine, uint32_t targetColumn) {
    zed_Piece *piece = buffer->pieces;
    uint64_t offset = 0;
    uint64_t line = 0;
    uint64_t column = 0;

    if (targetLine == 0 && targetColumn == 0) {
        buffer->cursor = 0;
        return;
    }

    while (NULL != piece) {
        for (uint64_t i = piece->start; i < piece->end; i++) {
            int8_t bufc = buffer->arena[i];

            if (bufc == '\n') {
                line += 1;
                column = 0;
            } else {
                column += 1;
            }

            offset += 1;
            if (line == targetLine && column == targetColumn) {
                buffer->cursor = offset;
                return;
            } else if (line > targetLine) {
                buffer->cursor = offset - 1;
                return;
            }
        }

        piece = piece->next;
    }
}

/**
 * zed_BufferMoveCursor moves the cursor in a given direction.
 */
zed_error_t zed_BufferMoveCursor(zed_Buffer *buffer, zed_CursorDirection direction) {
    uint64_t line, column;

    switch (direction) {
    case ZED_CURSOR_UP:
        zed_BufferComputePosition(buffer, &line, &column);
        zed_BufferGoTo(buffer, line - 1, column);
        break;

    case ZED_CURSOR_DOWN:
        zed_BufferComputePosition(buffer, &line, &column);
        zed_BufferGoTo(buffer, line + 1, column);
        break;

    case ZED_CURSOR_LEFT:
        if (buffer->cursor > 0) {
            buffer->cursor -= 1;
        }
        break;

    case ZED_CURSOR_RIGHT:
        if (buffer->cursor < zed_BufferLength(buffer)) {
            buffer->cursor += 1;
        }
        break;
    }

    return ZED_OK;
}

/**
 * zed_BufferComputePosition computes the cursor position.
 *
 * @param buffer The Buffer.
 * @param line A pointer to the address where the line number should be written.
 * @param column A pointer to the address where the column number should be written.
 */
zed_error_t zed_BufferComputePosition(zed_Buffer *buffer, uint64_t *line, uint64_t *column) {
    zed_Piece *piece = buffer->pieces;
    uint64_t offset = buffer->cursor;

    *line = 0;
    *column = 0;
    while (NULL != piece && 0 != offset) {
        for (uint64_t i = piece->start; i < piece->end; i++) {
            int8_t c = buffer->arena[i];

            if (c == '\n') {
                *line += 1;
                *column = 0;
            } else {
                *column += 1;
            }

            if (--offset == 0) {
                return ZED_OK;
            }
        }

        piece = piece->next;
    }

    return ZED_OK;
}

/**
 * zed_BufferLength computes the length of a buffer.
 */
uint64_t zed_BufferLength(zed_Buffer *buffer) {
    zed_Piece *piece = buffer->pieces;
    uint64_t length = 0;
    while (NULL != piece) {
        length += zed_PieceLength(piece);
        piece = piece->next;
    }

    return length;
}


/**
 * zed_BufferDump dumps a zed_Buffer to a string.
 *
 * @param buffer A pointer to the zed_Buffer.
 * @param output A pointer to the output buffer.
 */
void zed_BufferDump(zed_Buffer *buffer, int8_t *output) {
    zed_Piece *piece = buffer->pieces;
    while (NULL != piece) {
        uint64_t length = zed_PieceLength(piece);
        memcpy(output, buffer->arena + piece->start, length);
        output += length;
        piece = piece->next;
    }
}

/**
 * zed_BufferDelete frees a Buffer record.
 */
void zed_BufferDelete(zed_Buffer *buffer) {
    if (NULL != buffer->pieces) {
        zed_PieceDelete(buffer->pieces);
    }

    free(buffer->arena);
    free(buffer);
}
