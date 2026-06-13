#ifndef SFU_EXCEPTIONS_H
#define SFU_EXCEPTIONS_H

#include <stdint.h>

uint32_t sfu_exceptions_apply(int selector, uint32_t original, uint32_t reduced, uint32_t result);

#endif
