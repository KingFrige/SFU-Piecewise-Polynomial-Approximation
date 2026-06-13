#ifndef SFU_RECONSTRUCTION_H
#define SFU_RECONSTRUCTION_H

#include <stdint.h>

double sfu_reconstruction_bits_to_octave_double(uint32_t bits);
int sfu_reconstruction_value(uint32_t input,
                             int selector,
                             int active_selector,
                             uint32_t reduced,
                             double approx,
                             double *value);
uint32_t sfu_reconstruction_pack(double value);
uint32_t sfu_reconstruction_adjust_exp2(uint32_t reduced, uint32_t result);

#endif
