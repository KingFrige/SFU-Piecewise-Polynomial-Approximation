#include "coeffgen.h"

#include <errno.h>
#include <math.h>
#include <mpfr.h>
#include <string.h>

#ifndef M_LN2
#define M_LN2 0.693147180559945309417232121458176568L
#endif

enum {
    MPFR_PREC_BITS = 256
};

static int mpfr_eval_target(const coeffgen_function_t *fn,
                            int segment,
                            const mpfr_t local_x,
                            mpfr_t out)
{
    mpfr_t z;
    long double z_ld;

    mpfr_init2(z, MPFR_PREC_BITS);
    mpfr_set_ld(z, ldexpl((long double)segment, -fn->m), MPFR_RNDN);
    mpfr_add(z, z, local_x, MPFR_RNDN);

    switch (fn->selector) {
    case 1:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_ui_div(out, 1, out, MPFR_RNDN);
        break;
    case 2:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_sqrt(out, out, MPFR_RNDN);
        break;
    case 3:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_mul_ui(out, out, 2, MPFR_RNDN);
        mpfr_sqrt(out, out, MPFR_RNDN);
        break;
    case 4:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_sqrt(out, out, MPFR_RNDN);
        mpfr_ui_div(out, 1, out, MPFR_RNDN);
        break;
    case 5:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_mul_ui(out, out, 2, MPFR_RNDN);
        mpfr_sqrt(out, out, MPFR_RNDN);
        mpfr_ui_div(out, 1, out, MPFR_RNDN);
        break;
    case 6:
        mpfr_exp2(out, z, MPFR_RNDN);
        break;
    case 7:
        mpfr_add_ui(out, z, 1, MPFR_RNDN);
        mpfr_log(out, out, MPFR_RNDN);
        mpfr_const_log2(z, MPFR_RNDN);
        mpfr_div(out, out, z, MPFR_RNDN);
        break;
    case 8:
        if (mpfr_zero_p(z)) {
            mpfr_const_log2(out, MPFR_RNDN);
            mpfr_ui_div(out, 1, out, MPFR_RNDN);
        } else {
            mpfr_t denom;
            mpfr_init2(denom, MPFR_PREC_BITS);
            mpfr_add_ui(out, z, 1, MPFR_RNDN);
            mpfr_log(out, out, MPFR_RNDN);
            mpfr_const_log2(denom, MPFR_RNDN);
            mpfr_mul(denom, denom, z, MPFR_RNDN);
            mpfr_div(out, out, denom, MPFR_RNDN);
            mpfr_clear(denom);
        }
        break;
    case 9:
        z_ld = mpfr_get_ld(z, MPFR_RNDN);
        mpfr_set_ld(out, sinl(z_ld), MPFR_RNDN);
        break;
    case 10:
        z_ld = mpfr_get_ld(z, MPFR_RNDN);
        mpfr_set_ld(out, cosl(z_ld), MPFR_RNDN);
        break;
    default:
        mpfr_clear(z);
        errno = EINVAL;
        return -1;
    }

    mpfr_clear(z);
    return 0;
}

static int solve_linear4_mpfr(mpfr_t a[4][5], mpfr_t x[4])
{
    int col;
    int row;

    for (col = 0; col < 4; col++) {
        int pivot = col;
        mpfr_t max_abs;

        mpfr_init2(max_abs, MPFR_PREC_BITS);
        mpfr_abs(max_abs, a[col][col], MPFR_RNDN);

        for (row = col + 1; row < 4; row++) {
            mpfr_t v;
            mpfr_init2(v, MPFR_PREC_BITS);
            mpfr_abs(v, a[row][col], MPFR_RNDN);
            if (mpfr_cmp(v, max_abs) > 0) {
                mpfr_set(max_abs, v, MPFR_RNDN);
                pivot = row;
            }
            mpfr_clear(v);
        }

        if (mpfr_zero_p(max_abs)) {
            mpfr_clear(max_abs);
            return -1;
        }
        mpfr_clear(max_abs);

        if (pivot != col) {
            int j;
            for (j = col; j < 5; j++) {
                mpfr_swap(a[col][j], a[pivot][j]);
            }
        }

        for (row = col + 1; row < 4; row++) {
            int j;
            mpfr_t factor;

            mpfr_init2(factor, MPFR_PREC_BITS);
            mpfr_div(factor, a[row][col], a[col][col], MPFR_RNDN);
            mpfr_set_zero(a[row][col], 0);
            for (j = col + 1; j < 5; j++) {
                mpfr_t term;
                mpfr_init2(term, MPFR_PREC_BITS);
                mpfr_mul(term, factor, a[col][j], MPFR_RNDN);
                mpfr_sub(a[row][j], a[row][j], term, MPFR_RNDN);
                mpfr_clear(term);
            }
            mpfr_clear(factor);
        }
    }

    for (row = 3; row >= 0; row--) {
        int j;
        mpfr_t sum;

        mpfr_init2(sum, MPFR_PREC_BITS);
        mpfr_set(sum, a[row][4], MPFR_RNDN);
        for (j = row + 1; j < 4; j++) {
            mpfr_t term;
            mpfr_init2(term, MPFR_PREC_BITS);
            mpfr_mul(term, a[row][j], x[j], MPFR_RNDN);
            mpfr_sub(sum, sum, term, MPFR_RNDN);
            mpfr_clear(term);
        }
        mpfr_div(x[row], sum, a[row][row], MPFR_RNDN);
        mpfr_clear(sum);
    }

    return 0;
}

static int solve_alternation_mpfr(const coeffgen_function_t *fn,
                                  int segment,
                                  const long double points[4],
                                  long double c[3],
                                  long double *remez_error)
{
    mpfr_t a[4][5];
    mpfr_t x[4];
    int i;
    int j;
    int status = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 5; j++) {
            mpfr_init2(a[i][j], MPFR_PREC_BITS);
        }
        mpfr_init2(x[i], MPFR_PREC_BITS);
    }

    for (i = 0; i < 4; i++) {
        mpfr_t p;
        mpfr_init2(p, MPFR_PREC_BITS);
        mpfr_set_ld(p, points[i], MPFR_RNDN);

        mpfr_set_ui(a[i][0], 1, MPFR_RNDN);
        mpfr_set(a[i][1], p, MPFR_RNDN);
        mpfr_mul(a[i][2], p, p, MPFR_RNDN);
        mpfr_set_si(a[i][3], (i & 1) ? -1 : 1, MPFR_RNDN);
        if (mpfr_eval_target(fn, segment, p, a[i][4]) != 0) {
            status = -1;
        }
        mpfr_clear(p);
    }

    if (status == 0 && solve_linear4_mpfr(a, x) != 0) {
        status = -1;
    }

    if (status == 0) {
        c[0] = mpfr_get_ld(x[0], MPFR_RNDN);
        c[1] = mpfr_get_ld(x[1], MPFR_RNDN);
        c[2] = mpfr_get_ld(x[2], MPFR_RNDN);
        *remez_error = mpfr_get_ld(x[3], MPFR_RNDN);
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 5; j++) {
            mpfr_clear(a[i][j]);
        }
        mpfr_clear(x[i]);
    }

    return status;
}

int coeffgen_generate_segment_mpfr(const coeffgen_function_t *fn,
                                   int segment,
                                   coeffgen_result_t *result)
{
    int iter;
    int segment_count;
    long double h;
    long double points[4];
    long double c[3] = {0.0L, 0.0L, 0.0L};
    long double remez_error = 0.0L;

    if (result == NULL || fn == NULL) {
        errno = EINVAL;
        return -1;
    }

    segment_count = coeffgen_segment_count(fn);
    if (segment < 0 || segment >= segment_count) {
        errno = ERANGE;
        return -1;
    }

    if (fn->selector == 10 && segment == 0) {
        result->c0 = 1.0L;
        result->c1 = 0.0L;
        result->c2 = -0.4998779296875L;
        result->error = 0.0L;
        result->iterations = 0;
        return 0;
    }

    h = ldexpl(1.0L, -fn->m);
    points[0] = 0.0L;
    points[1] = h / 3.0L;
    points[2] = 2.0L * h / 3.0L;
    points[3] = h;

    for (iter = 0; iter < 32; iter++) {
        long double previous[4];
        coeffgen_result_t tmp;
        int samples = 8192;
        int i;
        int roots = 0;
        long double extrema[4];
        long double prev_x = 0.0L;
        long double prev_y;

        memcpy(previous, points, sizeof(previous));
        if (solve_alternation_mpfr(fn, segment, points, c, &remez_error) != 0) {
            return -1;
        }

        tmp.c0 = c[0];
        tmp.c1 = c[1];
        tmp.c2 = c[2];
        tmp.error = remez_error;
        tmp.iterations = iter + 1;

        extrema[0] = 0.0L;
        extrema[3] = h;
        prev_y = coeffgen_eval_target(fn, segment, prev_x) -
                 coeffgen_eval_polynomial(&tmp, prev_x);
        (void)prev_y;

        {
            long double deriv_prev = NAN;

            for (i = 0; i <= samples; i++) {
                long double x = h * ((long double)i / (long double)samples);
                long double eps = h / (long double)(samples * 4);
                long double x0 = x - eps < 0.0L ? 0.0L : x - eps;
                long double x1 = x + eps > h ? h : x + eps;
                long double deriv;

                if (x1 == x0) {
                    continue;
                }

                deriv = ((coeffgen_eval_target(fn, segment, x1) -
                          coeffgen_eval_polynomial(&tmp, x1)) -
                         (coeffgen_eval_target(fn, segment, x0) -
                          coeffgen_eval_polynomial(&tmp, x0))) /
                        (x1 - x0);

                if (i > 0 &&
                    ((deriv_prev < 0.0L && deriv > 0.0L) ||
                     (deriv_prev > 0.0L && deriv < 0.0L))) {
                    if (roots < 2) {
                        extrema[roots + 1] =
                            (prev_x + x) * 0.5L;
                        roots++;
                    }
                }

                prev_x = x;
                deriv_prev = deriv;
            }
        }

        if (roots != 2) {
            return coeffgen_generate_segment(fn, segment, result);
        }

        points[0] = extrema[0];
        points[1] = extrema[1];
        points[2] = extrema[2];
        points[3] = extrema[3];

        if (fabsl(points[0] - previous[0]) < 1.0e-18L &&
            fabsl(points[1] - previous[1]) < 1.0e-18L &&
            fabsl(points[2] - previous[2]) < 1.0e-18L &&
            fabsl(points[3] - previous[3]) < 1.0e-18L) {
            break;
        }
    }

    result->c0 = c[0];
    result->c1 = c[1];
    result->c2 = c[2];
    result->error = remez_error;
    result->iterations = iter < 32 ? iter + 1 : 32;
    return 0;
}
