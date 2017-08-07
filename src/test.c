#include <stdint.h>
#include <stdio.h>

#include "test.h"

failure_t failures[1024] = {};
uint64_t failureCount = 0;

void zed_MarkFailure(const char *filename, uint64_t line, const char *message) {
    failure_t failure = {message, filename, line};
    failures[failureCount++] = failure;
}

uint64_t zed_RunTests(UnitTest *tests, size_t testCount) {
    uint64_t failed = 0;
    for (size_t i = 0; i < testCount; i++) {
        printf(" * Running test %ld/%ld...\r", i + 1, testCount);
        if (0 != tests[i]()) {
            printf(" * Running test %ld/%ld... [fail]\n", i + 1, testCount);
            failed++;
        } else {
            printf(" * Running test %ld/%ld... [ok]\n", i + 1, testCount);
        }
    }

    for (size_t i = 0; i < failureCount; i++) {
        printf(" ----- %s:%llu: %s\n", failures[i].filename, failures[i].line, failures[i].message);
    }

    printf(" ================= success=%llu fail=%llu =================\n", testCount - failed, failed);
    return failed;
}
