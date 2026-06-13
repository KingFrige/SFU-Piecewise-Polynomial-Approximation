#include "sfu_rro.h"

#include "sfu_reconstruction.h"

#include <math.h>

static uint32_t fixed24_from_double(double value)
{
    double scaled = floor(value * 8388608.0);

    if (scaled <= 0.0) {
        return 0;
    }
    if (scaled >= 16777215.0) {
        return 0xffffffU;
    }

    return (uint32_t)scaled & 0xffffffU;
}

uint32_t sfu_rro_exp2(uint32_t input)
{
    uint32_t sign = input >> 31;
    uint32_t exp = (input >> 23) & 0xffU;
    uint32_t mant = input & 0x7fffffU;
    uint32_t base;
    uint32_t shifted;
    uint32_t adjusted;
    uint32_t shift;

    if (exp > 133U) {
        return 0xff800000U;
    }

    base = (1U << 29) | (mant << 6);
    shift = 133U - exp;
    shifted = shift >= 32U ? 0U : (base >> shift);
    adjusted = (shifted ^ (sign ? 0xffffffffU : 0U)) + sign;

    return adjusted & 0x7fffffffU;
}

uint32_t sfu_rro_trig(uint32_t input)
{
    static const double pi = 3.141592653589793238462643383279502884;
    double value = sfu_reconstruction_bits_to_octave_double(input);
    uint32_t sign;
    uint32_t quadrant;
    uint32_t fixed;

    if (value > 0.0) {
        sign = 0;
    } else {
        value = -value;
        sign = 1;
    }

    if (value > pi * 2.0) {
        double turns = floor(value / (pi * 2.0));
        value -= (pi * 2.0) * turns;
    }

    if (value > 0.0 && value <= pi / 2.0) {
        quadrant = 0;
    } else if (value > pi / 2.0 && value <= pi) {
        quadrant = 1;
        value -= pi / 2.0;
    } else if (value > pi && value <= (3.0 * pi) / 2.0) {
        quadrant = 2;
        value -= pi;
    } else {
        quadrant = 3;
        value -= (3.0 * pi) / 2.0;
    }

    fixed = fixed24_from_double(value);
    return (sign << 31) | (quadrant << 29) | fixed;
}
