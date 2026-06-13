#ifndef SFU_QUADRATIC_INTERPOLATOR_H
#define SFU_QUADRATIC_INTERPOLATOR_H

#include "sfu_internal.h"

#include <stdint.h>

int sfu_qi_active_selector(int selector, uint32_t reduced_input);
uint32_t sfu_qi_mantissa_for(int selector, uint32_t input, uint32_t reduced_input);
int sfu_qi_interpolate(const sfu_rom_table_t *table, uint32_t mantissa, int64_t *operation);

#endif
