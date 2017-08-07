#ifndef ZED_TEST_H
#define ZED_TEST_H

#include <stdint.h>

typedef enum {
    PASS,
    FAIL
} result_t;

typedef struct {
    const char *message;
    const char *filename;
    uint64_t line;
} failure_t;

typedef result_t (*UnitTest)();

#define fail(message) {zed_MarkFailure(__FILE__, __LINE__, #message); goto fail;}

void zed_MarkFailure(const char *, uint64_t, const char *);
uint64_t zed_RunTests(UnitTest *, size_t);

#endif
