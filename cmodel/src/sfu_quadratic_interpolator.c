#include "sfu_quadratic_interpolator.h"

int sfu_qi_active_selector(int selector, uint32_t reduced_input)
{
    uint32_t exp = (reduced_input >> 23) & 0xffU;
    uint32_t exp_lsb = exp & 1U;

    if (selector == 2) {
        return exp_lsb == 0 ? 3 : 2;
    }
    if (selector == 4) {
        return exp_lsb == 0 ? 5 : 4;
    }
    if (selector == 7) {
        return exp == 127 ? 8 : 7;
    }
    if (selector == 9) {
        uint32_t q0 = (reduced_input >> 29) & 1U;
        uint32_t over_one = (reduced_input >> 23) & 1U;
        int func_rro = q0 ? 10 : 9;

        if (func_rro == 9) {
            return over_one ? 10 : 9;
        }
        return over_one ? 9 : 10;
    }
    if (selector == 10) {
        uint32_t q0 = (reduced_input >> 29) & 1U;
        uint32_t over_one = (reduced_input >> 23) & 1U;
        int func_rro = q0 ? 9 : 10;

        if (func_rro == 10) {
            return over_one ? 9 : 10;
        }
        return over_one ? 10 : 9;
    }

    return selector;
}

uint32_t sfu_qi_mantissa_for(int selector, uint32_t input, uint32_t reduced_input)
{
    if (selector == 9 || selector == 10) {
        uint32_t fixed24 = reduced_input & 0xffffffU;

        if ((fixed24 & 0x800000U) != 0) {
            const uint32_t half_pi_fixed = 0xc90fdbU;
            return (half_pi_fixed - fixed24) & 0x7fffffU;
        }

        return fixed24 & 0x7fffffU;
    }
    if (selector == 6) {
        return reduced_input & 0x7fffffU;
    }

    return input & 0x7fffffU;
}

static int64_t sign_magnitude(uint32_t row, int width)
{
    uint32_t sign = row >> (width - 1);
    uint32_t magnitude = row & ((1U << (width - 1)) - 1U);

    return sign ? -(int64_t)magnitude : (int64_t)magnitude;
}

int sfu_qi_interpolate(const sfu_rom_table_t *table, uint32_t mantissa, int64_t *operation)
{
    int tail_bits;
    uint32_t segment;
    uint32_t tail;
    uint64_t square;
    int64_t c0;
    int64_t c1;
    int64_t c2;

    if (table == NULL || table->c0 == NULL || table->c1 == NULL ||
        table->c2 == NULL || operation == NULL) {
        return -1;
    }

    tail_bits = 23 - table->m;
    segment = mantissa >> tail_bits;
    if ((int)segment >= table->rows) {
        return -1;
    }
    tail = mantissa & ((1U << tail_bits) - 1U);
    square = ((uint64_t)tail * (uint64_t)tail) >> 19;

    c0 = sign_magnitude(table->c0[segment], SFU_C0_WIDTH);
    c1 = sign_magnitude(table->c1[segment], SFU_C1_WIDTH);
    c2 = sign_magnitude(table->c2[segment], SFU_C2_WIDTH);

    *operation = (c0 << 14) + c1 * (int64_t)tail + c2 * (int64_t)square * 2;
    return 0;
}
