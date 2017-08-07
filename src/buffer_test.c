#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "test.h"

result_t test_CanInsertTextIntoEmptyBuffer() {
    zed_Buffer *buff = zed_BufferCreate(NULL);
    char input[] = "hello!";
    char output[6] = {0};

    zed_error_t err = zed_BufferInsert(buff, 0, 6, (int8_t*)input);
    if (0 != err) {
        fail("could not perform insert");
    }

    zed_BufferDump(buff, (int8_t*)output);
    if (0 != memcmp(input, output, 6)) {
        fail("output is different");
    }

    zed_BufferDelete(buff);
    return PASS;

fail:
    zed_BufferDelete(buff);
    return FAIL;
}

result_t test_CanInsertTextAtEndOfBuffer() {
    zed_Buffer *buff = zed_BufferCreate(NULL);
    char output[13] = {0};

    zed_BufferInsert(buff, 0, 5, (int8_t*)"hello");
    zed_BufferInsert(buff, 7, 7, (int8_t*)" world!");
    zed_BufferDump(buff, (int8_t*)output);
    if (0 != memcmp("hello world!", output, 13)) {
        fail("output is different");
    }

    zed_BufferDelete(buff);
    return PASS;

fail:
    zed_BufferDelete(buff);
    return FAIL;
}

result_t test_CanInsertTextAtTheBeginning() {
    zed_Buffer *buff = zed_BufferCreate(NULL);
    char output[12] = {0};

    zed_BufferInsert(buff, 0, 5, (int8_t*)"world");
    zed_BufferInsert(buff, 0, 7, (int8_t*)"hello, ");
    zed_BufferDump(buff, (int8_t*)output);
    if (0 != memcmp("hello, world", output, 12)) {
        fail("output is different");
    }

    zed_BufferDelete(buff);
    return PASS;

fail:
    zed_BufferDelete(buff);
    return FAIL;
}


result_t test_CanInsertTextInTheMiddleOfBuffer() {
    zed_Buffer *buff = zed_BufferCreate(NULL);
    char output[17] = {0};

    zed_BufferInsert(buff, 0, 13, (int8_t*)"hello, world!");
    zed_BufferInsert(buff, 7, 6, (int8_t*)"cruel ");
    zed_BufferDump(buff, (int8_t*)output);
    if (0 != memcmp("hello, cruel world!", output, 19)) {
        printf("output=%s\n", output);
        fail("output is different");
    }

    if (19 != zed_BufferLength(buff)) {
        fail("buffer has the wrong length");
    }

    zed_BufferDelete(buff);
    return PASS;

fail:
    zed_BufferDelete(buff);
    return FAIL;
}


int main(void) {
    UnitTest tests[] = {
        &test_CanInsertTextIntoEmptyBuffer,
        &test_CanInsertTextAtEndOfBuffer,
        &test_CanInsertTextAtTheBeginning,
        &test_CanInsertTextInTheMiddleOfBuffer,
    };

    return zed_RunTests(tests, sizeof(tests) / sizeof(UnitTest));
}
