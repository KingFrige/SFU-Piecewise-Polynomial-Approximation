#include "sfu.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SFU_TABLE_COUNT 11
#define SFU_ERR_SIZE 256

typedef struct {
    int selector;
    const char *name;
} function_desc_t;

typedef struct {
    int selector;
    char name[64];
    int m;
    int rows;
    int width0;
    int width1;
    int width2;
    uint32_t *c0;
    uint32_t *c1;
    uint32_t *c2;
} lut_table_t;

struct sfu_luts {
    lut_table_t table[SFU_TABLE_COUNT];
};

static const function_desc_t k_functions[] = {
    {1, "reci"},
    {2, "sqrt_1_2"},
    {3, "sqrt_2_4"},
    {4, "reci_sqrt_1_2"},
    {5, "reci_sqrt_2_4"},
    {6, "exp"},
    {7, "ln2"},
    {8, "ln2e0"},
    {9, "sin"},
    {10, "cos"},
};

static void set_error(char *err, size_t err_size, const char *fmt, ...)
{
    va_list ap;

    if (err == NULL || err_size == 0) {
        return;
    }

    va_start(ap, fmt);
    vsnprintf(err, err_size, fmt, ap);
    va_end(ap);
}

const char *sfu_selector_name(int selector)
{
    size_t i;

    for (i = 0; i < sizeof(k_functions) / sizeof(k_functions[0]); i++) {
        if (k_functions[i].selector == selector) {
            return k_functions[i].name;
        }
    }

    return NULL;
}

int sfu_selector_supported(int selector)
{
    return selector == 1 || selector == 2 || selector == 4 || selector == 6 ||
           selector == 7 || selector == 9 || selector == 10;
}

int sfu_parse_hex32(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;

    if (text == NULL || value == NULL || text[0] == '\0') {
        return -1;
    }

    errno = 0;
    parsed = strtoul(text, &end, 16);
    if (errno != 0 || end == text || *end != '\0' || parsed > 0xffffffffUL) {
        return -1;
    }

    *value = (uint32_t)parsed;
    return 0;
}

static int make_path(char *out, size_t out_size, const char *root, const char *dir, const char *file)
{
    int n = snprintf(out, out_size, "%s/%s/%s", root, dir, file);

    return n < 0 || n >= (int)out_size ? -1 : 0;
}

static int strip_line(char *line)
{
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }

    return (int)len;
}

static int parse_binary_row(const char *line, int width, uint32_t *value)
{
    int i;
    uint32_t parsed = 0;

    if ((int)strlen(line) != width || width <= 0 || width > 31) {
        return -1;
    }

    for (i = 0; i < width; i++) {
        if (line[i] != '0' && line[i] != '1') {
            return -1;
        }
        parsed = (parsed << 1) | (uint32_t)(line[i] - '0');
    }

    *value = parsed;
    return 0;
}

static int parse_metadata(const char *path, lut_table_t *table, char *err, size_t err_size)
{
    FILE *fp;
    char line[256];

    fp = fopen(path, "r");
    if (fp == NULL) {
        set_error(err, err_size, "failed to open metadata %s", path);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char key[128];
        char value[128];

        strip_line(line);
        if (sscanf(line, "%127[^=]=%127s", key, value) != 2) {
            continue;
        }

        if (strcmp(key, "selector") == 0) {
            table->selector = atoi(value);
        } else if (strcmp(key, "name") == 0) {
            strncpy(table->name, value, sizeof(table->name) - 1);
            table->name[sizeof(table->name) - 1] = '\0';
        } else if (strcmp(key, "m") == 0) {
            table->m = atoi(value);
        } else if (strcmp(key, "rows") == 0) {
            table->rows = atoi(value);
        } else if (strcmp(key, "LUTC0_width") == 0) {
            table->width0 = atoi(value);
        } else if (strcmp(key, "LUTC1_width") == 0) {
            table->width1 = atoi(value);
        } else if (strcmp(key, "LUTC2_width") == 0) {
            table->width2 = atoi(value);
        }
    }

    fclose(fp);

    if (table->selector <= 0 || table->selector >= SFU_TABLE_COUNT ||
        table->m <= 0 || table->rows <= 0 ||
        table->width0 != 29 || table->width1 != 20 || table->width2 != 14) {
        set_error(err, err_size, "invalid metadata in %s", path);
        return -1;
    }

    return 0;
}

static int load_lut_file(const char *path,
                         int width,
                         int rows,
                         uint32_t **out,
                         char *err,
                         size_t err_size)
{
    FILE *fp;
    uint32_t *values;
    char line[256];
    int row = 0;

    values = (uint32_t *)calloc((size_t)rows, sizeof(*values));
    if (values == NULL) {
        set_error(err, err_size, "out of memory loading %s", path);
        return -1;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        free(values);
        set_error(err, err_size, "failed to open LUT %s", path);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        strip_line(line);
        if (row >= rows) {
            fclose(fp);
            free(values);
            set_error(err, err_size, "too many rows in %s", path);
            return -1;
        }
        if (parse_binary_row(line, width, &values[row]) != 0) {
            fclose(fp);
            free(values);
            set_error(err, err_size, "invalid row %d in %s", row + 1, path);
            return -1;
        }
        row++;
    }

    fclose(fp);

    if (row != rows) {
        free(values);
        set_error(err, err_size, "expected %d rows in %s, got %d", rows, path, row);
        return -1;
    }

    *out = values;
    return 0;
}

static int load_one_table(const char *root,
                          const function_desc_t *desc,
                          lut_table_t *table,
                          char *err,
                          size_t err_size)
{
    char dir[128];
    char path[4096];

    memset(table, 0, sizeof(*table));
    snprintf(dir, sizeof(dir), "%02d_%s", desc->selector, desc->name);

    if (make_path(path, sizeof(path), root, dir, "metadata.txt") != 0 ||
        parse_metadata(path, table, err, err_size) != 0) {
        return -1;
    }

    if (table->selector != desc->selector || strcmp(table->name, desc->name) != 0) {
        set_error(err, err_size, "metadata mismatch in %s", path);
        return -1;
    }

    if (make_path(path, sizeof(path), root, dir, "LUTC0.txt") != 0 ||
        load_lut_file(path, table->width0, table->rows, &table->c0, err, err_size) != 0) {
        return -1;
    }
    if (make_path(path, sizeof(path), root, dir, "LUTC1.txt") != 0 ||
        load_lut_file(path, table->width1, table->rows, &table->c1, err, err_size) != 0) {
        return -1;
    }
    if (make_path(path, sizeof(path), root, dir, "LUTC2.txt") != 0 ||
        load_lut_file(path, table->width2, table->rows, &table->c2, err, err_size) != 0) {
        return -1;
    }

    return 0;
}

void sfu_free_luts(sfu_luts_t *luts)
{
    int i;

    if (luts == NULL) {
        return;
    }

    for (i = 0; i < SFU_TABLE_COUNT; i++) {
        free(luts->table[i].c0);
        free(luts->table[i].c1);
        free(luts->table[i].c2);
    }
    free(luts);
}

int sfu_load_luts(const char *root,
                         sfu_luts_t **out,
                         char *err,
                         size_t err_size)
{
    sfu_luts_t *luts;
    size_t i;

    if (root == NULL || out == NULL) {
        set_error(err, err_size, "invalid LUT load arguments");
        return -1;
    }

    luts = (sfu_luts_t *)calloc(1, sizeof(*luts));
    if (luts == NULL) {
        set_error(err, err_size, "out of memory");
        return -1;
    }

    for (i = 0; i < sizeof(k_functions) / sizeof(k_functions[0]); i++) {
        const int selector = k_functions[i].selector;

        if (load_one_table(root, &k_functions[i], &luts->table[selector], err, err_size) != 0) {
            sfu_free_luts(luts);
            return -1;
        }
    }

    *out = luts;
    return 0;
}

static double bits_to_octave_double(uint32_t bits)
{
    int sign = (bits >> 31) != 0;
    int exp = (int)((bits >> 23) & 0xff) - 127;
    uint32_t mant = bits & 0x7fffffU;
    double value = (1.0 + (double)mant / 8388608.0) * ldexp(1.0, exp);

    return sign ? -value : value;
}

static uint32_t double_to_octave_hex32(double value)
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

static uint32_t reduce_exp2(uint32_t input)
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

static uint32_t reduce_trig(uint32_t input)
{
    static const double pi = 3.141592653589793238462643383279502884;
    double value = bits_to_octave_double(input);
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

static int active_selector_for(int selector, uint32_t reduced_input)
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

static int64_t sign_magnitude(uint32_t row, int width)
{
    uint32_t sign = row >> (width - 1);
    uint32_t magnitude = row & ((1U << (width - 1)) - 1U);

    return sign ? -(int64_t)magnitude : (int64_t)magnitude;
}

static int interpolate(const lut_table_t *table, uint32_t mantissa, int64_t *operation)
{
    int tail_bits;
    uint32_t segment;
    uint32_t tail;
    uint64_t square;
    int64_t c0;
    int64_t c1;
    int64_t c2;

    if (table == NULL || table->c0 == NULL || table->c1 == NULL || table->c2 == NULL) {
        return -1;
    }

    tail_bits = 23 - table->m;
    segment = mantissa >> tail_bits;
    if ((int)segment >= table->rows) {
        return -1;
    }
    tail = mantissa & ((1U << tail_bits) - 1U);
    square = ((uint64_t)tail * (uint64_t)tail) >> 19;

    c0 = sign_magnitude(table->c0[segment], 29);
    c1 = sign_magnitude(table->c1[segment], 20);
    c2 = sign_magnitude(table->c2[segment], 14);

    *operation = (c0 << 14) + c1 * (int64_t)tail + c2 * (int64_t)square * 2;
    return 0;
}

static uint32_t trig_mantissa(uint32_t reduced_input)
{
    uint32_t fixed24 = reduced_input & 0xffffffU;

    if ((fixed24 & 0x800000U) != 0) {
        const uint32_t half_pi_fixed = 0xc90fdbU;
        return (half_pi_fixed - fixed24) & 0x7fffffU;
    }

    return fixed24 & 0x7fffffU;
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

static uint32_t apply_basic_exceptions(int selector, uint32_t original, uint32_t reduced, uint32_t result)
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

int sfu_eval_hex(const sfu_luts_t *luts,
                        uint32_t input,
                        int selector,
                        uint32_t *result,
                        char *err,
                        size_t err_size)
{
    uint32_t reduced = input;
    uint32_t mantissa;
    uint32_t exp = (input >> 23) & 0xffU;
    int active_selector;
    const lut_table_t *table;
    int64_t operation;
    double approx;
    double value;

    if (luts == NULL || result == NULL) {
        set_error(err, err_size, "invalid eval arguments");
        return -1;
    }
    if (!sfu_selector_supported(selector)) {
        set_error(err, err_size, "unsupported selector %d; supported: 1,2,4,6,7,9,10", selector);
        return -1;
    }

    if (selector == 6) {
        reduced = reduce_exp2(input);
    } else if (selector == 9 || selector == 10) {
        reduced = reduce_trig(input);
    }

    active_selector = active_selector_for(selector, reduced);
    table = &luts->table[active_selector];

    if (selector == 6 || selector == 9 || selector == 10) {
        if (selector == 9 || selector == 10) {
            mantissa = trig_mantissa(reduced);
        } else {
            mantissa = reduced & 0x7fffffU;
        }
    } else {
        mantissa = input & 0x7fffffU;
    }

    if (interpolate(table, mantissa, &operation) != 0) {
        set_error(err, err_size, "failed interpolation for selector %d", active_selector);
        return -1;
    }

    approx = ldexp((double)operation, -41);

    if (selector == 1) {
        int unbiased = (int)exp - 127;
        value = ldexp(approx, -unbiased);
        if ((input >> 31) != 0) {
            value = -value;
        }
    } else if (selector == 2) {
        int unbiased = (int)exp - 127;
        if (active_selector == 2) {
            value = approx * pow(2.0, (double)unbiased / 2.0);
        } else {
            value = approx * pow(2.0, (double)(unbiased - 1) / 2.0);
        }
    } else if (selector == 4) {
        int unbiased = (int)exp - 127;
        if (active_selector == 4) {
            value = approx * pow(2.0, -(double)unbiased / 2.0);
        } else {
            value = approx * pow(2.0, -(double)(unbiased - 1) / 2.0);
        }
    } else if (selector == 6) {
        uint32_t field = (reduced >> 23) & 0xffU;
        int exp2;

        if ((reduced & 0x40000000U) != 0) {
            exp2 = -((int)(((~field) & 0xffU) + 1U));
        } else {
            exp2 = (int)field;
        }
        value = ldexp(approx, exp2);
    } else if (selector == 7) {
        if (active_selector == 8) {
            value = approx * (bits_to_octave_double(input) - 1.0);
        } else {
            value = approx + ((int)exp - 127);
        }
    } else if (selector == 9 || selector == 10) {
        value = trig_result_negative(selector, reduced) ? -approx : approx;
    } else {
        set_error(err, err_size, "unsupported selector %d", selector);
        return -1;
    }

    *result = double_to_octave_hex32(value);

    if (selector == 6) {
        uint32_t r_sign = reduced >> 31;
        uint32_t r_exp = (reduced >> 23) & 0xffU;
        if (r_sign != 0 && r_exp == 0) {
            *result = 0x00000000U;
        } else if (r_sign != 0 && r_exp == 0xffU) {
            *result = reduced & 0x7fffffffU;
        }
    }

    *result = apply_basic_exceptions(selector, input, reduced, *result);
    return 0;
}
