#include "coeffgen.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int test_pdf_exp_example(void)
{
    const coeffgen_function_t *fn = coeffgen_find_function("exp");
    coeffgen_result_t r;
    long double dc0;
    long double dc1;
    long double dc2;

    if (fn == NULL || coeffgen_generate_segment(fn, 0, &r) != 0) {
        fprintf(stderr, "failed to generate PDF exp example\n");
        return 1;
    }

    dc0 = fabsl(r.c0 - 1.000000006650307888975275342165L);
    dc1 = fabsl(r.c1 - 0.69313952400640021838596581052087L);
    dc2 = fabsl(r.c2 - 0.24153165203330618309424664972368L);

    if (dc0 > 1.0e-10L || dc1 > 1.0e-8L || dc2 > 1.0e-8L) {
        fprintf(stderr,
                "PDF exp example mismatch: dc0=%.6Le dc1=%.6Le dc2=%.6Le\n",
                dc0,
                dc1,
                dc2);
        return 1;
    }

    return 0;
}

static int test_function_metadata(void)
{
    const char *expected_names[] = {
        "reci",
        "sqrt_1_2",
        "sqrt_2_4",
        "reci_sqrt_1_2",
        "reci_sqrt_2_4",
        "exp",
        "ln2",
        "ln2e0",
        "sin",
        "cos",
    };
    const int expected_m[] = {7, 6, 6, 7, 7, 6, 7, 6, 6, 6};
    const int expected_t[] = {26, 25, 25, 26, 26, 25, 26, 26, 27, 27};
    const int expected_p[] = {16, 15, 15, 16, 16, 15, 15, 15, 18, 18};
    const int expected_q[] = {10, 11, 11, 10, 10, 11, 10, 10, 13, 13};
    size_t count = coeffgen_function_count();
    size_t i;

    if (count != 10) {
        fprintf(stderr, "expected 10 coefficient functions, got %zu\n", count);
        return 1;
    }

    for (i = 0; i < count; i++) {
        const coeffgen_function_t *fn = coeffgen_function_at(i);
        int rows;

        if (fn == NULL ||
            fn->selector != (int)i + 1 ||
            strcmp(fn->name, expected_names[i]) != 0 ||
            fn->m != expected_m[i] ||
            fn->t != expected_t[i] ||
            fn->p != expected_p[i] ||
            fn->q != expected_q[i]) {
            fprintf(stderr, "metadata mismatch at function index %zu\n", i);
            return 1;
        }

        rows = coeffgen_segment_count(fn);
        if (rows != (1 << fn->m)) {
            fprintf(stderr, "%s expected %d rows, got %d\n", fn->name, 1 << fn->m, rows);
            return 1;
        }
    }

    return 0;
}

static int test_variant_metadata(void)
{
    coeffgen_variant_t variant;

    if (coeffgen_variant_count() != 5) {
        fprintf(stderr, "expected 5 variants, got %zu\n", coeffgen_variant_count());
        return 1;
    }

    if (coeffgen_parse_variant("remez-compat", &variant) != 0 ||
        variant != COEFFGEN_VARIANT_REMEZ_COMPAT ||
        strcmp(coeffgen_variant_name(variant), "remez-compat") != 0) {
        fprintf(stderr, "failed to parse remez-compat variant\n");
        return 1;
    }

    if (coeffgen_parse_variant("pdf-quant", &variant) != 0 ||
        variant != COEFFGEN_VARIANT_PDF_QUANT ||
        !coeffgen_variant_available(variant)) {
        fprintf(stderr, "failed to parse pdf-quant variant\n");
        return 1;
    }

    if (coeffgen_parse_variant("pdf-hw-search", &variant) != 0 ||
        variant != COEFFGEN_VARIANT_PDF_HW_SEARCH ||
        !coeffgen_variant_available(variant)) {
        fprintf(stderr, "failed to parse pdf-hw-search variant\n");
        return 1;
    }

    if (coeffgen_parse_variant("not-a-variant", &variant) == 0) {
        fprintf(stderr, "unexpectedly parsed invalid variant\n");
        return 1;
    }

    return 0;
}

static int test_pdf_quant_exp_example(void)
{
    const coeffgen_function_t *fn = coeffgen_find_function("exp");
    coeffgen_result_t r;

    if (fn == NULL ||
        coeffgen_generate_segment_variant(fn, 0, COEFFGEN_VARIANT_PDF_QUANT, &r) != 0) {
        fprintf(stderr, "failed to generate PDF quantized exp example\n");
        return 1;
    }

    if (fabsl(r.c0 - 1.0L) > 1.0e-18L ||
        fabsl(r.c1 - 0.693145751953125L) > 1.0e-18L ||
        fabsl(r.c2 - 0.2412109375L) > 1.0e-18L) {
        fprintf(stderr,
                "PDF quantized exp mismatch: c0=%.18Le c1=%.18Le c2=%.18Le\n",
                r.c0,
                r.c1,
                r.c2);
        return 1;
    }

    return 0;
}

static int test_pdf_hw_search_exp_example(void)
{
    const coeffgen_function_t *fn = coeffgen_find_function("exp");
    coeffgen_result_t pdf;
    coeffgen_result_t search;
    coeffgen_error_stats_t pdf_stats;
    coeffgen_error_stats_t search_stats;

    if (fn == NULL ||
        coeffgen_generate_segment_variant(fn, 0, COEFFGEN_VARIANT_PDF_QUANT, &pdf) != 0 ||
        coeffgen_generate_segment_variant(fn, 0, COEFFGEN_VARIANT_PDF_HW_SEARCH, &search) != 0 ||
        coeffgen_evaluate_segment_hw_error(fn, 0, &pdf, 256, &pdf_stats) != 0 ||
        coeffgen_evaluate_segment_hw_error(fn, 0, &search, 256, &search_stats) != 0) {
        fprintf(stderr, "failed to compare PDF hardware search exp example\n");
        return 1;
    }

    if (search_stats.max_abs_error > pdf_stats.max_abs_error + 1.0e-18L) {
        fprintf(stderr,
                "pdf-hw-search regressed exp hardware error: pdf=%.18Le search=%.18Le\n",
                pdf_stats.max_abs_error,
                search_stats.max_abs_error);
        return 1;
    }

    return 0;
}

static int test_generate_all_segments(void)
{
    size_t i;
    int total_rows = 0;

    for (i = 0; i < coeffgen_function_count(); i++) {
        const coeffgen_function_t *fn = coeffgen_function_at(i);
        int rows = coeffgen_segment_count(fn);
        int segment;

        for (segment = 0; segment < rows; segment++) {
            coeffgen_result_t r;

            if (coeffgen_generate_segment(fn, segment, &r) != 0 ||
                !isfinite((double)r.c0) ||
                !isfinite((double)r.c1) ||
                !isfinite((double)r.c2)) {
                fprintf(stderr, "generation failed for %s segment %d\n", fn->name, segment);
                return 1;
            }
        }

        printf("%2d %-16s rows=%3d\n", fn->selector, fn->name, rows);
        total_rows += rows;
    }

    if (total_rows != 896) {
        fprintf(stderr, "expected 896 total coefficient rows, got %d\n", total_rows);
        return 1;
    }

    printf("Generated %d coefficient rows in C-only tests.\n", total_rows);
    return 0;
}

int main(void)
{
    if (test_pdf_exp_example() != 0) {
        return 1;
    }

    if (test_function_metadata() != 0) {
        return 1;
    }

    if (test_variant_metadata() != 0) {
        return 1;
    }

    if (test_pdf_quant_exp_example() != 0) {
        return 1;
    }
    if (test_pdf_hw_search_exp_example() != 0) {
        return 1;
    }

    return test_generate_all_segments();
}
