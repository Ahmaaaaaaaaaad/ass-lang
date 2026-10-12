#ifndef __SPAN_H__
#define __SPAN_H__

#include <stdint.h>

typedef struct
{
    uint32_t offset;
    uint16_t len;
} span_t;

#endif
