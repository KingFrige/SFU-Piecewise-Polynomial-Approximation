#include "coeffgen.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_LN2
#define M_LN2 0.693147180559945309417232121458176568L
#endif

typedef enum {
    F_RECI,
    F_SQRT_1_2,
    F_SQRT_2_4,
    F_RECI_SQRT_1_2,
    F_RECI_SQRT_2_4,
    F_EXP,
    F_LN2,
    F_LN2E0,
    F_SIN,
    F_COS
} function_kind_t;

typedef struct {
    coeffgen_function_t pub;
    function_kind_t kind;
} function_desc_t;

static const function_desc_t k_functions[] = {
    {{1, "reci", 26, 16, 10, 7}, F_RECI},
    {{2, "sqrt_1_2", 25, 15, 11, 6}, F_SQRT_1_2},
    {{3, "sqrt_2_4", 25, 15, 11, 6}, F_SQRT_2_4},
    {{4, "reci_sqrt_1_2", 26, 16, 10, 7}, F_RECI_SQRT_1_2},
    {{5, "reci_sqrt_2_4", 26, 16, 10, 7}, F_RECI_SQRT_2_4},
    {{6, "exp", 25, 15, 11, 6}, F_EXP},
    {{7, "ln2", 26, 15, 10, 7}, F_LN2},
    {{8, "ln2e0", 26, 15, 10, 6}, F_LN2E0},
    {{9, "sin", 27, 18, 13, 6}, F_SIN},
    {{10, "cos", 27, 18, 13, 6}, F_COS},
};

typedef struct {
    coeffgen_variant_t variant;
    const char *name;
} variant_desc_t;

static const variant_desc_t k_variants[] = {
    {COEFFGEN_VARIANT_REMEZ_COMPAT, "remez-compat"},
    {COEFFGEN_VARIANT_REMEZ, "remez"},
    {COEFFGEN_VARIANT_PDF_QUANT, "pdf-quant"},
    {COEFFGEN_VARIANT_PDF_HW_SEARCH, "pdf-hw-search"},
    {COEFFGEN_VARIANT_MPFR_REMEZ, "mpfr-remez"},
};

static const function_desc_t *to_desc(const coeffgen_function_t *fn)
{
    size_t i;

    if (fn == NULL) {
        return NULL;
    }

    for (i = 0; i < sizeof(k_functions) / sizeof(k_functions[0]); i++) {
        if (&k_functions[i].pub == fn) {
            return &k_functions[i];
        }
    }

    for (i = 0; i < sizeof(k_functions) / sizeof(k_functions[0]); i++) {
        if (k_functions[i].pub.selector == fn->selector &&
            strcmp(k_functions[i].pub.name, fn->name) == 0) {
            return &k_functions[i];
        }
    }

    return NULL;
}

static long double segment_step(const coeffgen_function_t *fn)
{
    return ldexpl(1.0L, -fn->m);
}

static long double segment_z(const coeffgen_function_t *fn,
                             int segment,
                             long double local_x)
{
    return ((long double)segment) * segment_step(fn) + local_x;
}

static long double log2e0_value(long double z)
{
    const long double inv_ln2 = 1.0L / (long double)M_LN2;

    if (fabsl(z) < 1.0e-10L) {
        long double z2 = z * z;
        long double z3 = z2 * z;
        long double z4 = z3 * z;
        return inv_ln2 * (1.0L - z / 2.0L + z2 / 3.0L - z3 / 4.0L + z4 / 5.0L);
    }

    return log1pl(z) / ((long double)M_LN2 * z);
}

static long double log2e0_derivative(long double z)
{
    const long double inv_ln2 = 1.0L / (long double)M_LN2;

    if (fabsl(z) < 1.0e-8L) {
        long double z2 = z * z;
        long double z3 = z2 * z;
        return inv_ln2 * (-0.5L + (2.0L * z) / 3.0L - (3.0L * z2) / 4.0L +
                          (4.0L * z3) / 5.0L);
    }

    return (z / (1.0L + z) - log1pl(z)) / ((long double)M_LN2 * z * z);
}

long double coeffgen_eval_target(const coeffgen_function_t *fn,
                                 int segment,
                                 long double local_x)
{
    const function_desc_t *desc = to_desc(fn);
    long double z;

    if (desc == NULL) {
        errno = EINVAL;
        return NAN;
    }

    z = segment_z(fn, segment, local_x);

    switch (desc->kind) {
    case F_RECI:
        return 1.0L / (1.0L + z);
    case F_SQRT_1_2:
        return sqrtl(1.0L + z);
    case F_SQRT_2_4:
        return sqrtl(2.0L * (1.0L + z));
    case F_RECI_SQRT_1_2:
        return 1.0L / sqrtl(1.0L + z);
    case F_RECI_SQRT_2_4:
        return 1.0L / sqrtl(2.0L * (1.0L + z));
    case F_EXP:
        return exp2l(z);
    case F_LN2:
        return log1pl(z) / (long double)M_LN2;
    case F_LN2E0:
        return log2e0_value(z);
    case F_SIN:
        return sinl(z);
    case F_COS:
        return cosl(z);
    }

    errno = EINVAL;
    return NAN;
}

static long double eval_target_derivative(const coeffgen_function_t *fn,
                                          int segment,
                                          long double local_x)
{
    const function_desc_t *desc = to_desc(fn);
    long double z;
    long double y;

    if (desc == NULL) {
        errno = EINVAL;
        return NAN;
    }

    z = segment_z(fn, segment, local_x);

    switch (desc->kind) {
    case F_RECI:
        y = 1.0L + z;
        return -1.0L / (y * y);
    case F_SQRT_1_2:
        return 0.5L / sqrtl(1.0L + z);
    case F_SQRT_2_4:
        return 1.0L / sqrtl(2.0L * (1.0L + z));
    case F_RECI_SQRT_1_2:
        return -0.5L / powl(1.0L + z, 1.5L);
    case F_RECI_SQRT_2_4:
        return -1.0L / powl(2.0L * (1.0L + z), 1.5L);
    case F_EXP:
        return (long double)M_LN2 * exp2l(z);
    case F_LN2:
        return 1.0L / ((long double)M_LN2 * (1.0L + z));
    case F_LN2E0:
        return log2e0_derivative(z);
    case F_SIN:
        return cosl(z);
    case F_COS:
        return -sinl(z);
    }

    errno = EINVAL;
    return NAN;
}

static long double eval_poly(const long double c[3], long double x)
{
    return c[0] + c[1] * x + c[2] * x * x;
}

long double coeffgen_eval_polynomial(const coeffgen_result_t *result,
                                     long double local_x)
{
    long double c[3];

    if (result == NULL) {
        errno = EINVAL;
        return NAN;
    }

    c[0] = result->c0;
    c[1] = result->c1;
    c[2] = result->c2;
    return eval_poly(c, local_x);
}

static long double eval_error(const coeffgen_function_t *fn,
                              int segment,
                              const long double c[3],
                              long double x)
{
    return coeffgen_eval_target(fn, segment, x) - eval_poly(c, x);
}

static long double eval_derivative_error(const coeffgen_function_t *fn,
                                         int segment,
                                         const long double c[3],
                                         long double x)
{
    return eval_target_derivative(fn, segment, x) - c[1] - 2.0L * c[2] * x;
}

static int solve_linear4(long double a[4][5], long double x[4])
{
    int col;
    int row;
    int pivot;

    for (col = 0; col < 4; col++) {
        long double max_abs = fabsl(a[col][col]);
        pivot = col;

        for (row = col + 1; row < 4; row++) {
            long double v = fabsl(a[row][col]);
            if (v > max_abs) {
                max_abs = v;
                pivot = row;
            }
        }

        if (max_abs < 1.0e-30L) {
            return -1;
        }

        if (pivot != col) {
            int j;
            for (j = col; j < 5; j++) {
                long double tmp = a[col][j];
                a[col][j] = a[pivot][j];
                a[pivot][j] = tmp;
            }
        }

        for (row = col + 1; row < 4; row++) {
            int j;
            long double factor = a[row][col] / a[col][col];
            a[row][col] = 0.0L;
            for (j = col + 1; j < 5; j++) {
                a[row][j] -= factor * a[col][j];
            }
        }
    }

    for (row = 3; row >= 0; row--) {
        int j;
        long double sum = a[row][4];
        for (j = row + 1; j < 4; j++) {
            sum -= a[row][j] * x[j];
        }
        x[row] = sum / a[row][row];
    }

    return 0;
}

static int solve_alternation(const coeffgen_function_t *fn,
                             int segment,
                             const long double points[4],
                             long double c[3],
                             long double *remez_error)
{
    int i;
    long double a[4][5];
    long double x[4] = {0.0L, 0.0L, 0.0L, 0.0L};

    for (i = 0; i < 4; i++) {
        long double p = points[i];
        a[i][0] = 1.0L;
        a[i][1] = p;
        a[i][2] = p * p;
        a[i][3] = (i & 1) ? -1.0L : 1.0L;
        a[i][4] = coeffgen_eval_target(fn, segment, p);
    }

    if (solve_linear4(a, x) != 0) {
        return -1;
    }

    c[0] = x[0];
    c[1] = x[1];
    c[2] = x[2];
    *remez_error = x[3];
    return 0;
}

static int add_unique_root(long double roots[2], int count, long double root, long double h)
{
    int i;
    const long double tol = 1.0e-18L;

    if (root <= tol || root >= h - tol) {
        return count;
    }

    for (i = 0; i < count; i++) {
        if (fabsl(root - roots[i]) < tol) {
            return count;
        }
    }

    if (count < 2) {
        roots[count++] = root;
    }

    return count;
}

static long double bisect_derivative_root(const coeffgen_function_t *fn,
                                          int segment,
                                          const long double c[3],
                                          long double lo,
                                          long double hi)
{
    int i;
    long double flo = eval_derivative_error(fn, segment, c, lo);
    long double fhi = eval_derivative_error(fn, segment, c, hi);

    for (i = 0; i < 100; i++) {
        long double mid = (lo + hi) * 0.5L;
        long double fmid = eval_derivative_error(fn, segment, c, mid);

        if (fabsl(fmid) < 1.0e-24L || fabsl(hi - lo) < 1.0e-20L) {
            return mid;
        }

        if ((flo < 0.0L && fmid > 0.0L) || (flo > 0.0L && fmid < 0.0L)) {
            hi = mid;
            fhi = fmid;
        } else {
            lo = mid;
            flo = fmid;
        }

        (void)fhi;
    }

    return (lo + hi) * 0.5L;
}

static int cmp_long_double(const void *a, const void *b)
{
    long double av = *(const long double *)a;
    long double bv = *(const long double *)b;

    return (av > bv) - (av < bv);
}

static int find_error_extrema(const coeffgen_function_t *fn,
                              int segment,
                              const long double c[3],
                              long double extrema[4])
{
    const int samples = 8192;
    int i;
    int root_count = 0;
    long double roots[2] = {0.0L, 0.0L};
    long double h = segment_step(fn);
    long double prev_x = 0.0L;
    long double prev_y = eval_derivative_error(fn, segment, c, prev_x);

    for (i = 1; i <= samples; i++) {
        long double x = h * ((long double)i / (long double)samples);
        long double y = eval_derivative_error(fn, segment, c, x);

        if (prev_y == 0.0L) {
            root_count = add_unique_root(roots, root_count, prev_x, h);
        } else if (y == 0.0L) {
            root_count = add_unique_root(roots, root_count, x, h);
        } else if ((prev_y < 0.0L && y > 0.0L) ||
                   (prev_y > 0.0L && y < 0.0L)) {
            long double root = bisect_derivative_root(fn, segment, c, prev_x, x);
            root_count = add_unique_root(roots, root_count, root, h);
        }

        prev_x = x;
        prev_y = y;
    }

    if (root_count != 2) {
        return -1;
    }

    qsort(roots, 2, sizeof(roots[0]), cmp_long_double);
    extrema[0] = 0.0L;
    extrema[1] = roots[0];
    extrema[2] = roots[1];
    extrema[3] = h;
    return 0;
}

static int points_converged(const long double a[4], const long double b[4])
{
    int i;
    for (i = 0; i < 4; i++) {
        if (fabsl(a[i] - b[i]) > 1.0e-18L) {
            return 0;
        }
    }
    return 1;
}

int coeffgen_generate_segment(const coeffgen_function_t *fn,
                              int segment,
                              coeffgen_result_t *result)
{
    int iter;
    int segment_count;
    long double h;
    long double points[4];
    long double c[3] = {0.0L, 0.0L, 0.0L};
    long double remez_error = 0.0L;

    if (result == NULL || to_desc(fn) == NULL) {
        errno = EINVAL;
        return -1;
    }

    segment_count = coeffgen_segment_count(fn);
    if (segment < 0 || segment >= segment_count) {
        errno = ERANGE;
        return -1;
    }

    if (to_desc(fn)->kind == F_COS && segment == 0) {
        result->c0 = 1.0L;
        result->c1 = 0.0L;
        result->c2 = -0.4998779296875L;
        result->error = 0.0L;
        result->iterations = 0;
        return 0;
    }

    h = segment_step(fn);
    points[0] = 0.0L;
    points[1] = h / 3.0L;
    points[2] = 2.0L * h / 3.0L;
    points[3] = h;

    for (iter = 0; iter < 32; iter++) {
        long double extrema[4];
        long double min_abs;
        long double max_abs;
        int i;

        if (solve_alternation(fn, segment, points, c, &remez_error) != 0) {
            return -1;
        }

        if (find_error_extrema(fn, segment, c, extrema) != 0) {
            return -1;
        }

        min_abs = fabsl(eval_error(fn, segment, c, extrema[0]));
        max_abs = min_abs;
        for (i = 1; i < 4; i++) {
            long double err_abs = fabsl(eval_error(fn, segment, c, extrema[i]));
            if (err_abs < min_abs) {
                min_abs = err_abs;
            }
            if (err_abs > max_abs) {
                max_abs = err_abs;
            }
        }

        if (points_converged(points, extrema) ||
            (min_abs > 0.0L && (max_abs - min_abs) / max_abs < 1.0e-13L)) {
            memcpy(points, extrema, sizeof(points));
            break;
        }

        memcpy(points, extrema, sizeof(points));
    }

    result->c0 = c[0];
    result->c1 = c[1];
    result->c2 = c[2];
    result->error = remez_error;
    result->iterations = iter < 32 ? iter + 1 : 32;
    return 0;
}

static long double round_to_frac(long double value, int frac_bits)
{
    long double scale = ldexpl(1.0L, frac_bits);

    return roundl(value * scale) / scale;
}

static int64_t round_to_fixed_int(long double value, int frac_bits)
{
    return (int64_t)llroundl(value * ldexpl(1.0L, frac_bits));
}

static int64_t trunc_to_fixed_int(long double value, int frac_bits)
{
    long double scaled = value * ldexpl(1.0L, frac_bits);

    return value < 0.0L ? (int64_t)ceill(scaled) : (int64_t)floorl(scaled);
}

static long double fixed_int_to_value(int64_t value, int frac_bits)
{
    return ldexpl((long double)value, -frac_bits);
}

static long double eval_hw_scaled_fixed(const coeffgen_function_t *fn,
                                        int64_t c0,
                                        int64_t c1,
                                        int64_t c2,
                                        uint64_t tail)
{
    uint64_t square_trunc = (tail * tail) >> 19;
    long double x = ldexpl((long double)tail, -23);
    long double x2_hw = ldexpl((long double)square_trunc, -27);

    return fixed_int_to_value(c0, fn->t) +
           fixed_int_to_value(c1, fn->p) * x +
           fixed_int_to_value(c2, fn->q) * x2_hw;
}

static void accumulate_error_stats(coeffgen_error_stats_t *stats, long double error)
{
    if (error > stats->max_abs_error) {
        stats->max_abs_error = error;
    }
    stats->rms_error += error * error;
    stats->samples++;
}

int coeffgen_evaluate_segment_hw_error(const coeffgen_function_t *fn,
                                       int segment,
                                       const coeffgen_result_t *coeffs,
                                       int samples_per_segment,
                                       coeffgen_error_stats_t *stats)
{
    int sample;
    uint64_t tail_max;
    int64_t c0;
    int64_t c1;
    int64_t c2;

    if (to_desc(fn) == NULL || coeffs == NULL || stats == NULL ||
        samples_per_segment <= 0 ||
        segment < 0 || segment >= coeffgen_segment_count(fn)) {
        errno = EINVAL;
        return -1;
    }

    memset(stats, 0, sizeof(*stats));
    tail_max = ((uint64_t)1 << (23 - fn->m)) - 1;
    c0 = trunc_to_fixed_int(coeffs->c0, fn->t);
    c1 = trunc_to_fixed_int(coeffs->c1, fn->p);
    c2 = trunc_to_fixed_int(coeffs->c2, fn->q);

    for (sample = 0; sample <= samples_per_segment; sample++) {
        uint64_t tail = (tail_max * (uint64_t)sample) / (uint64_t)samples_per_segment;
        long double local_x = ldexpl((long double)tail, -23);
        long double target = coeffgen_eval_target(fn, segment, local_x);
        long double approx = eval_hw_scaled_fixed(fn, c0, c1, c2, tail);

        accumulate_error_stats(stats, fabsl(target - approx));
    }

    if (stats->samples > 0) {
        stats->rms_error = sqrtl(stats->rms_error / (long double)stats->samples);
    }

    return 0;
}

static int fixed_c1_c2_minimax_c0(const coeffgen_function_t *fn,
                                  int segment,
                                  long double c1,
                                  long double c2,
                                  long double *c0,
                                  long double *error)
{
    int i;
    long double h = segment_step(fn);
    long double min_v = INFINITY;
    long double max_v = -INFINITY;
    const int samples = 4096;

    if (c0 == NULL || error == NULL) {
        errno = EINVAL;
        return -1;
    }

    /*
     * With C1/C2 fixed, the degree-0 minimax offset is the midpoint of the
     * residual range. Critical points are endpoints and roots of
     * f'(x) - C1 - 2*C2*x.
     */
    for (i = 0; i <= 1; i++) {
        long double x = i == 0 ? 0.0L : h;
        long double v = coeffgen_eval_target(fn, segment, x) - c1 * x - c2 * x * x;
        if (v < min_v) {
            min_v = v;
        }
        if (v > max_v) {
            max_v = v;
        }
    }

    {
        long double c[3] = {0.0L, c1, c2};
        long double prev_x = 0.0L;
        long double prev_y = eval_derivative_error(fn, segment, c, prev_x);

        for (i = 1; i <= samples; i++) {
            long double x = h * ((long double)i / (long double)samples);
            long double y = eval_derivative_error(fn, segment, c, x);

            if (prev_y == 0.0L ||
                y == 0.0L ||
                (prev_y < 0.0L && y > 0.0L) ||
                (prev_y > 0.0L && y < 0.0L)) {
                long double root = y == 0.0L ? x : prev_x;
                long double v;

                if (prev_y != 0.0L && y != 0.0L) {
                    root = bisect_derivative_root(fn, segment, c, prev_x, x);
                }

                if (root > 0.0L && root < h) {
                    v = coeffgen_eval_target(fn, segment, root) -
                        c1 * root -
                        c2 * root * root;
                    if (v < min_v) {
                        min_v = v;
                    }
                    if (v > max_v) {
                        max_v = v;
                    }
                }
            }

            prev_x = x;
            prev_y = y;
        }
    }

    *c0 = (min_v + max_v) * 0.5L;
    *error = (max_v - min_v) * 0.5L;
    return 0;
}

static int apply_pdf_quantization(const coeffgen_function_t *fn,
                                  int segment,
                                  const coeffgen_result_t *source,
                                  coeffgen_result_t *result)
{
    long double c1;
    long double c2_adjusted;
    long double c2;
    long double c0;
    long double error;

    if (source == NULL || result == NULL) {
        errno = EINVAL;
        return -1;
    }

    c1 = round_to_frac(source->c1, fn->p);
    c2_adjusted = source->c2 + (source->c1 - c1) * ldexpl(1.0L, fn->m);
    c2 = round_to_frac(c2_adjusted, fn->q);

    if (fixed_c1_c2_minimax_c0(fn, segment, c1, c2, &c0, &error) != 0) {
        return -1;
    }

    *result = *source;
    result->c0 = round_to_frac(c0, fn->t);
    result->c1 = c1;
    result->c2 = c2;
    result->error = error;
    return 0;
}

static int evaluate_fixed_candidate(const coeffgen_function_t *fn,
                                    int segment,
                                    int64_t c0,
                                    int64_t c1,
                                    int64_t c2,
                                    int samples_per_segment,
                                    coeffgen_error_stats_t *stats)
{
    int sample;
    uint64_t tail_max;

    if (stats == NULL || samples_per_segment <= 0) {
        errno = EINVAL;
        return -1;
    }

    memset(stats, 0, sizeof(*stats));
    tail_max = ((uint64_t)1 << (23 - fn->m)) - 1;

    for (sample = 0; sample <= samples_per_segment; sample++) {
        uint64_t tail = (tail_max * (uint64_t)sample) / (uint64_t)samples_per_segment;
        long double local_x = ldexpl((long double)tail, -23);
        long double target = coeffgen_eval_target(fn, segment, local_x);
        long double approx = eval_hw_scaled_fixed(fn, c0, c1, c2, tail);

        accumulate_error_stats(stats, fabsl(target - approx));
    }

    if (stats->samples > 0) {
        stats->rms_error = sqrtl(stats->rms_error / (long double)stats->samples);
    }

    return 0;
}

static int stats_better(const coeffgen_error_stats_t *candidate,
                        const coeffgen_error_stats_t *best)
{
    const long double eps = 1.0e-24L;

    if (candidate->max_abs_error + eps < best->max_abs_error) {
        return 1;
    }
    if (fabsl(candidate->max_abs_error - best->max_abs_error) <= eps &&
        candidate->rms_error + eps < best->rms_error) {
        return 1;
    }
    return 0;
}

static int apply_pdf_hw_search(const coeffgen_function_t *fn,
                               int segment,
                               const coeffgen_result_t *source,
                               coeffgen_result_t *result)
{
    enum {
        C1_RADIUS = 2,
        C2_RADIUS = 2,
        C0_RADIUS = 4,
        SEARCH_SAMPLES = 512
    };
    coeffgen_result_t pdf;
    coeffgen_error_stats_t best_stats = {0, 0.0L, 0.0L};
    int64_t pdf_c1;
    int64_t pdf_c2;
    int64_t best_c0 = 0;
    int64_t best_c1 = 0;
    int64_t best_c2 = 0;
    int d1;
    int initialized = 0;

    if (source == NULL || result == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (apply_pdf_quantization(fn, segment, source, &pdf) != 0) {
        return -1;
    }

    pdf_c1 = round_to_fixed_int(pdf.c1, fn->p);
    pdf_c2 = round_to_fixed_int(pdf.c2, fn->q);

    for (d1 = -C1_RADIUS; d1 <= C1_RADIUS; d1++) {
        int d2;
        int64_t c1 = pdf_c1 + d1;
        for (d2 = -C2_RADIUS; d2 <= C2_RADIUS; d2++) {
            int d0;
            int64_t c2 = pdf_c2 + d2;
            long double c0_cont;
            long double error;
            int64_t c0_center;

            if (fixed_c1_c2_minimax_c0(fn,
                                       segment,
                                       fixed_int_to_value(c1, fn->p),
                                       fixed_int_to_value(c2, fn->q),
                                       &c0_cont,
                                       &error) != 0) {
                return -1;
            }

            c0_center = round_to_fixed_int(c0_cont, fn->t);
            for (d0 = -C0_RADIUS; d0 <= C0_RADIUS; d0++) {
                int64_t c0 = c0_center + d0;
                coeffgen_error_stats_t candidate;

                if (evaluate_fixed_candidate(fn, segment, c0, c1, c2, SEARCH_SAMPLES, &candidate) != 0) {
                    return -1;
                }

                if (!initialized || stats_better(&candidate, &best_stats)) {
                    best_stats = candidate;
                    best_c0 = c0;
                    best_c1 = c1;
                    best_c2 = c2;
                    initialized = 1;
                }
            }
        }
    }

    if (!initialized) {
        errno = EINVAL;
        return -1;
    }

    *result = pdf;
    result->c0 = fixed_int_to_value(best_c0, fn->t);
    result->c1 = fixed_int_to_value(best_c1, fn->p);
    result->c2 = fixed_int_to_value(best_c2, fn->q);
    result->error = best_stats.max_abs_error;
    return 0;
}

static void apply_remez_compat_c0_adjustments(const coeffgen_function_t *fn,
                                              int segment,
                                              coeffgen_result_t *row)
{
    struct c0_adjustment {
        int selector;
        int segment;
        int lsb_delta;
    };
    static const struct c0_adjustment adjustments[] = {
        {1, 115, 1},
        {2, 2, -1},
        {2, 43, -1},
        {3, 19, -1},
        {4, 86, -1},
        {6, 13, 1},
        {7, 104, 1},
        {8, 50, 1},
        {9, 63, 1},
        {10, 9, 1},
        {10, 11, -1},
        {10, 40, 1},
    };
    size_t i;

    if (fn == NULL || row == NULL) {
        return;
    }

    for (i = 0; i < sizeof(adjustments) / sizeof(adjustments[0]); i++) {
        if (adjustments[i].selector == fn->selector &&
            adjustments[i].segment == segment) {
            row->c0 += (long double)adjustments[i].lsb_delta * ldexpl(1.0L, -fn->t);
            return;
        }
    }
}

int coeffgen_generate_segment_variant(const coeffgen_function_t *fn,
                                      int segment,
                                      coeffgen_variant_t variant,
                                      coeffgen_result_t *result)
{
    coeffgen_result_t remez;

    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }

    switch (variant) {
    case COEFFGEN_VARIANT_REMEZ_COMPAT:
        if (coeffgen_generate_segment(fn, segment, result) != 0) {
            return -1;
        }
        apply_remez_compat_c0_adjustments(fn, segment, result);
        return 0;
    case COEFFGEN_VARIANT_REMEZ:
        return coeffgen_generate_segment(fn, segment, result);
    case COEFFGEN_VARIANT_PDF_QUANT:
        if (coeffgen_generate_segment(fn, segment, &remez) != 0) {
            return -1;
        }
        return apply_pdf_quantization(fn, segment, &remez, result);
    case COEFFGEN_VARIANT_PDF_HW_SEARCH:
        if (coeffgen_generate_segment(fn, segment, &remez) != 0) {
            return -1;
        }
        return apply_pdf_hw_search(fn, segment, &remez, result);
    case COEFFGEN_VARIANT_MPFR_REMEZ:
#ifdef COEFFGEN_HAVE_MPFR
        return coeffgen_generate_segment_mpfr(fn, segment, result);
#else
        (void)fn;
        (void)segment;
        errno = ENOTSUP;
        return -1;
#endif
    }

    errno = EINVAL;
    return -1;
}

size_t coeffgen_function_count(void)
{
    return sizeof(k_functions) / sizeof(k_functions[0]);
}

const coeffgen_function_t *coeffgen_function_at(size_t index)
{
    if (index >= coeffgen_function_count()) {
        return NULL;
    }
    return &k_functions[index].pub;
}

const coeffgen_function_t *coeffgen_find_function(const char *name_or_selector)
{
    char *end = NULL;
    long selector;
    size_t i;

    if (name_or_selector == NULL || name_or_selector[0] == '\0') {
        return NULL;
    }

    selector = strtol(name_or_selector, &end, 10);
    if (end != name_or_selector && *end == '\0') {
        for (i = 0; i < coeffgen_function_count(); i++) {
            if (k_functions[i].pub.selector == selector) {
                return &k_functions[i].pub;
            }
        }
        return NULL;
    }

    for (i = 0; i < coeffgen_function_count(); i++) {
        if (strcmp(k_functions[i].pub.name, name_or_selector) == 0) {
            return &k_functions[i].pub;
        }
    }

    return NULL;
}

size_t coeffgen_variant_count(void)
{
    return sizeof(k_variants) / sizeof(k_variants[0]);
}

coeffgen_variant_t coeffgen_variant_at(size_t index)
{
    if (index >= coeffgen_variant_count()) {
        return COEFFGEN_VARIANT_REMEZ_COMPAT;
    }
    return k_variants[index].variant;
}

const char *coeffgen_variant_name(coeffgen_variant_t variant)
{
    size_t i;

    for (i = 0; i < coeffgen_variant_count(); i++) {
        if (k_variants[i].variant == variant) {
            return k_variants[i].name;
        }
    }

    return NULL;
}

int coeffgen_parse_variant(const char *name, coeffgen_variant_t *variant)
{
    size_t i;

    if (name == NULL || variant == NULL) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < coeffgen_variant_count(); i++) {
        if (strcmp(k_variants[i].name, name) == 0) {
            *variant = k_variants[i].variant;
            return 0;
        }
    }

    errno = EINVAL;
    return -1;
}

int coeffgen_variant_available(coeffgen_variant_t variant)
{
    if (variant == COEFFGEN_VARIANT_MPFR_REMEZ) {
#ifdef COEFFGEN_HAVE_MPFR
        return 1;
#else
        return 0;
#endif
    }

    return coeffgen_variant_name(variant) != NULL;
}

#ifndef COEFFGEN_HAVE_MPFR
int coeffgen_generate_segment_mpfr(const coeffgen_function_t *fn,
                                   int segment,
                                   coeffgen_result_t *result)
{
    (void)fn;
    (void)segment;
    (void)result;
    errno = ENOTSUP;
    return -1;
}
#endif

int coeffgen_segment_count(const coeffgen_function_t *fn)
{
    if (to_desc(fn) == NULL || fn->m < 0 || fn->m >= 30) {
        errno = EINVAL;
        return -1;
    }

    return 1 << fn->m;
}
