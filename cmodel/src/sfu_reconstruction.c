#include "sfu_reconstruction.h"

#include <math.h>
#include <stddef.h>

double sfu_reconstruction_bits_to_octave_double(uint32_t bits)
{
    int sign = (bits >> 31) != 0;
    int exp = (int)((bits >> 23) & 0xff) - 127;
    uint32_t mant = bits & 0x7fffffU;
    double value = (1.0 + (double)mant / 8388608.0) * ldexp(1.0, exp);

    return sign ? -value : value;
}

static int trig_result_negative(int selector, uint32_t reduced_input)
{
    uint32_t sign = reduced_input >> 31;
    uint32_t q1 = (reduced_input >> 30) & 1U;
    uint32_t q0 = (reduced_input >> 29) & 1U;

    if (selector == 9) {
        return (int)(sign ^ q1);
    }

    return (int)(q1 ^ q0);
}

int sfu_reconstruction_value(uint32_t input,
                             int selector,
                             int active_selector,
                             uint32_t reduced,
                             double approx,
                             double *value)
{
    uint32_t exp = (input >> 23) & 0xffU;

    if (value == NULL) {
        return -1;
    }

    if (selector == 1) {
        int unbiased = (int)exp - 127;
        *value = ldexp(approx, -unbiased);
        if ((input >> 31) != 0) {
            *value = -*value;
        }
    } else if (selector == 2) {
        int unbiased = (int)exp - 127;
        if (active_selector == 2) {
            *value = approx * pow(2.0, (double)unbiased / 2.0);
        } else {
            *value = approx * pow(2.0, (double)(unbiased - 1) / 2.0);
        }
    } else if (selector == 4) {
        int unbiased = (int)exp - 127;
        if (active_selector == 4) {
            *value = approx * pow(2.0, -(double)unbiased / 2.0);
        } else {
            *value = approx * pow(2.0, -(double)(unbiased - 1) / 2.0);
        }
    } else if (selector == 6) {
        uint32_t field = (reduced >> 23) & 0xffU;
        int exp2;

        if ((reduced & 0x40000000U) != 0) {
            exp2 = -((int)(((~field) & 0xffU) + 1U));
        } else {
            exp2 = (int)field;
        }
        *value = ldexp(approx, exp2);
    } else if (selector == 7) {
        if (active_selector == 8) {
            *value = approx * (sfu_reconstruction_bits_to_octave_double(input) - 1.0);
        } else {
            *value = approx + ((int)exp - 127);
        }
    } else if (selector == 9 || selector == 10) {
        *value = trig_result_negative(selector, reduced) ? -approx : approx;
    } else {
        return -1;
    }

    return 0;
}

uint32_t sfu_reconstruction_pack(double value)
{
    int sign;
    long double abs_value;
    int exp2;
    long double scaled;
    long double frac;
    uint64_t mant25;
    uint32_t mant23;
    int exp_bits;

    if (value == 0.0) {
        return signbit(value) ? 0x80000000U : 0x00000000U;
    }
    if (isnan(value)) {
        return 0xffffffffU;
    }
    if (isinf(value)) {
        return signbit(value) ? 0xff800000U : 0x7f800000U;
    }

    sign = signbit(value) ? 1 : 0;
    abs_value = sign ? -(long double)value : (long double)value;
    exp2 = (int)floorl(log2l(abs_value));
    scaled = ldexpl(abs_value, -exp2);
    if (scaled < 1.0L) {
        scaled *= 2.0L;
        exp2--;
    } else if (scaled >= 2.0L) {
        scaled *= 0.5L;
        exp2++;
    }

    frac = scaled - 1.0L;
    mant25 = (uint64_t)floorl(frac * 33554432.0L);
    mant23 = (uint32_t)(mant25 >> 2);
    if (((mant25 >> 1) & 1U) != 0) {
        mant23++;
    }
    if (mant23 >= 0x800000U) {
        mant23 = 0;
        exp2++;
    }

    exp_bits = exp2 + 127;
    if (exp_bits <= 0) {
        return ((uint32_t)sign << 31) | mant23;
    }
    if (exp_bits >= 255) {
        return ((uint32_t)sign << 31) | 0x7f800000U;
    }

    return ((uint32_t)sign << 31) | ((uint32_t)exp_bits << 23) | mant23;
}

uint32_t sfu_reconstruction_adjust_exp2(uint32_t reduced, uint32_t result)
{
    uint32_t r_sign = reduced >> 31;
    uint32_t r_exp = (reduced >> 23) & 0xffU;

    if (r_sign != 0 && r_exp == 0) {
        return 0x00000000U;
    }
    if (r_sign != 0 && r_exp == 0xffU) {
        return reduced & 0x7fffffffU;
    }

    return result;
}
