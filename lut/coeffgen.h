#ifndef SFU_LUT_COEFFGEN_H
#define SFU_LUT_COEFFGEN_H

#include <stddef.h>

typedef struct {
    int selector;
    const char *name;
    int t;
    int p;
    int q;
    int m;
} coeffgen_function_t;

typedef struct {
    long double c0;
    long double c1;
    long double c2;
    long double error;
    int iterations;
} coeffgen_result_t;

typedef enum {
    COEFFGEN_VARIANT_REMEZ_COMPAT = 0,
    COEFFGEN_VARIANT_REMEZ,
    COEFFGEN_VARIANT_PDF_QUANT,
    COEFFGEN_VARIANT_MPFR_REMEZ
} coeffgen_variant_t;

size_t coeffgen_function_count(void);
const coeffgen_function_t *coeffgen_function_at(size_t index);
const coeffgen_function_t *coeffgen_find_function(const char *name_or_selector);

size_t coeffgen_variant_count(void);
coeffgen_variant_t coeffgen_variant_at(size_t index);
const char *coeffgen_variant_name(coeffgen_variant_t variant);
int coeffgen_parse_variant(const char *name, coeffgen_variant_t *variant);
int coeffgen_variant_available(coeffgen_variant_t variant);

int coeffgen_segment_count(const coeffgen_function_t *fn);
int coeffgen_generate_segment(const coeffgen_function_t *fn,
                              int segment,
                              coeffgen_result_t *result);
int coeffgen_generate_segment_variant(const coeffgen_function_t *fn,
                                      int segment,
                                      coeffgen_variant_t variant,
                                      coeffgen_result_t *result);
int coeffgen_generate_segment_mpfr(const coeffgen_function_t *fn,
                                   int segment,
                                   coeffgen_result_t *result);
long double coeffgen_eval_target(const coeffgen_function_t *fn,
                                 int segment,
                                 long double local_x);
long double coeffgen_eval_polynomial(const coeffgen_result_t *result,
                                     long double local_x);

#endif
