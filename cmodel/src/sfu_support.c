#include "sfu_internal.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static const sfu_op_desc_t k_sfu_ops[] = {
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

void sfu_set_error(char *err, size_t err_size, const char *fmt, ...)
{
    va_list ap;

    if (err == NULL || err_size == 0) {
        return;
    }

    va_start(ap, fmt);
    vsnprintf(err, err_size, fmt, ap);
    va_end(ap);
}

const sfu_op_desc_t *sfu_ops_all(size_t *count)
{
    if (count != NULL) {
        *count = sizeof(k_sfu_ops) / sizeof(k_sfu_ops[0]);
    }
    return k_sfu_ops;
}

const char *sfu_selector_name(int selector)
{
    size_t i;

    for (i = 0; i < sizeof(k_sfu_ops) / sizeof(k_sfu_ops[0]); i++) {
        if (k_sfu_ops[i].selector == selector) {
            return k_sfu_ops[i].name;
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
