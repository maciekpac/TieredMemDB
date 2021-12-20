#pragma once

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tracker_handle tracker_handle;

extern tracker_handle *tracker_create(void);
extern void tracker_destroy(tracker_handle *h);

extern void tracker_add(tracker_handle *h, size_t size);

#ifdef __cplusplus
}
#endif
