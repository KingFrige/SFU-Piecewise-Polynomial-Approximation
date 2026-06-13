#include "sfu_exceptions.h"

uint32_t sfu_exceptions_apply(int selector, uint32_t original, uint32_t reduced, uint32_t result)
{
    uint32_t sign = original >> 31;
    uint32_t exp = (original >> 23) & 0xffU;
    uint32_t mant = original & 0x7fffffU;

    if (selector == 9 || selector == 10) {
        if (exp == 0xffU) {
            return 0xffffffffU;
        }
        if ((original & 0x7fffffffU) == 0 && (reduced & 0x00ffffffU) == 0) {
            if (selector == 9) {
                return result & 0x80000000U;
            }
            return (result & 0x80000000U) | 0x3f800000U;
        }
        return result;
    }

    if (selector == 4) {
        if (sign) {
            return exp == 0 ? 0xff800000U : 0xffffffffU;
        }
        if (exp == 0) {
            return 0x7f800000U;
        }
        if (exp == 0xffU) {
            return mant == 0 ? 0x00000000U : 0xffffffffU;
        }
        return result;
    }

    if (selector == 7) {
        if (exp == 0) {
            return 0xff800000U;
        }
        if (exp == 0xffU) {
            return sign == 0 && mant == 0 ? 0x7f800000U : 0xffffffffU;
        }
        if (sign) {
            return 0xffffffffU;
        }
        return result;
    }

    if (selector == 1) {
        if (sign) {
            if (exp == 0xffU && mant == 0) {
                return 0x80000000U;
            }
            if (exp == 0xffU) {
                return original;
            }
            if (exp == 0) {
                return 0xff800000U;
            }
        } else {
            if (exp == 0xffU && mant == 0) {
                return 0x00000000U;
            }
            if (exp == 0xffU) {
                return original;
            }
            if (exp == 0) {
                return 0x7f800000U;
            }
        }
        return result;
    }

    if (selector == 2) {
        if (sign) {
            if (exp == 0) {
                return 0xff800000U;
            }
            return 0xffffffffU;
        }
        if (exp == 0xffU) {
            return mant == 0 ? 0x7f800000U : 0xffffffffU;
        }
        if (exp == 0) {
            return 0x00000000U;
        }
        return result;
    }

    (void)reduced;
    return result;
}
