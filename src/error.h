#ifndef ZED_ERROR_H
#define ZED_ERROR_H

#include <stdint.h>

typedef enum {
    ZED_OK = 0,
    ZED_ALLOC_FAILED,
    ZED_FILE_NOT_FOUND,
} zed_error_t;

#endif
