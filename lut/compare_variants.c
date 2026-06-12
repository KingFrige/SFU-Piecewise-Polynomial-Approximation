#include "coeffgen.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    int width;
    long rows;
    long mismatches;
    unsigned long long max_diff;
} bit_stats_t;

typedef struct {
    long samples;
    long double max_abs_error;
    long double rms_error_accum;
} error_stats_t;

typedef struct {
    error_stats_t source;
    error_stats_t fixed;
    error_stats_t hardware;
} variant_error_stats_t;

static int read_line(FILE *fp, char *buf, size_t size)
{
    size_t len;

    if (fgets(buf, (int)size, fp) == NULL) {
        return 0;
    }

    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }

    return 1;
}

static unsigned long long parse_bits(const char *line, int width)
{
    unsigned long long value = 0;
    int i;

    for (i = 0; i < width; i++) {
        if (line[i] != '0' && line[i] != '1') {
            return 0;
        }
        value = (value << 1) | (unsigned long long)(line[i] - '0');
    }

    return value;
}

static void format_dir(char *out, size_t out_size, const coeffgen_function_t *fn)
{
    if (snprintf(out, out_size, "%02d_%s", fn->selector, fn->name) >= (int)out_size) {
        fprintf(stderr, "directory name too long for %s\n", fn->name);
        exit(1);
    }
}

static void join_path(char *out, size_t out_size, const char *root, const char *dir, const char *file)
{
    if (snprintf(out, out_size, "%s/%s/%s", root, dir, file) >= (int)out_size) {
        fprintf(stderr, "path too long: %s/%s/%s\n", root, dir, file);
        exit(1);
    }
}

static int compare_lut_file(const char *reference_root,
                            const char *variant_root,
                            const coeffgen_function_t *fn,
                            bit_stats_t *stats)
{
    char dir[128];
    char reference_path[4096];
    char variant_path[4096];
    FILE *reference;
    FILE *variant;
    char reference_line[128];
    char variant_line[128];
    int row = 0;

    format_dir(dir, sizeof(dir), fn);
    join_path(reference_path, sizeof(reference_path), reference_root, dir, stats->name);
    join_path(variant_path, sizeof(variant_path), variant_root, dir, stats->name);

    reference = fopen(reference_path, "r");
    variant = fopen(variant_path, "r");
    if (reference == NULL || variant == NULL) {
        fprintf(stderr, "failed to open %s or %s: %s\n", reference_path, variant_path, strerror(errno));
        if (reference != NULL) {
            fclose(reference);
        }
        if (variant != NULL) {
            fclose(variant);
        }
        return 1;
    }

    while (read_line(reference, reference_line, sizeof(reference_line))) {
        unsigned long long reference_value;
        unsigned long long variant_value;
        unsigned long long diff;

        row++;
        if (!read_line(variant, variant_line, sizeof(variant_line))) {
            fprintf(stderr, "%s ended before row %d\n", variant_path, row);
            fclose(reference);
            fclose(variant);
            return 1;
        }

        if ((int)strlen(reference_line) != stats->width ||
            (int)strlen(variant_line) != stats->width) {
            fprintf(stderr, "%s row %d has unexpected width\n", stats->name, row);
            fclose(reference);
            fclose(variant);
            return 1;
        }

        reference_value = parse_bits(reference_line, stats->width);
        variant_value = parse_bits(variant_line, stats->width);
        diff = reference_value > variant_value ? reference_value - variant_value : variant_value - reference_value;

        if (diff != 0) {
            stats->mismatches++;
        }
        if (diff > stats->max_diff) {
            stats->max_diff = diff;
        }
        stats->rows++;
    }

    if (read_line(variant, variant_line, sizeof(variant_line))) {
        fprintf(stderr, "%s has extra rows\n", variant_path);
        fclose(reference);
        fclose(variant);
        return 1;
    }

    fclose(reference);
    fclose(variant);
    return 0;
}

static long double trunc_to_frac(long double value, int frac_bits)
{
    long double scale = ldexpl(1.0L, frac_bits);
    long double abs_value = value < 0.0L ? -value : value;
    long double fixed = floorl(abs_value * scale) / scale;

    return value < 0.0L ? -fixed : fixed;
}

static void accumulate_error(error_stats_t *stats, long double error)
{
    if (error > stats->max_abs_error) {
        stats->max_abs_error = error;
    }
    stats->rms_error_accum += error * error;
    stats->samples++;
}

static void quantize_coefficients(const coeffgen_function_t *fn,
                                  const coeffgen_result_t *source,
                                  coeffgen_result_t *fixed)
{
    *fixed = *source;
    fixed->c0 = trunc_to_frac(source->c0, fn->t);
    fixed->c1 = trunc_to_frac(source->c1, fn->p);
    fixed->c2 = trunc_to_frac(source->c2, fn->q);
}

static int compute_error_stats(coeffgen_variant_t variant, variant_error_stats_t *stats)
{
    const int samples_per_segment = 16;
    const int hardware_samples_per_segment = 256;
    size_t i;

    memset(stats, 0, sizeof(*stats));

    for (i = 0; i < coeffgen_function_count(); i++) {
        const coeffgen_function_t *fn = coeffgen_function_at(i);
        int rows = coeffgen_segment_count(fn);
        int segment;

        for (segment = 0; segment < rows; segment++) {
            coeffgen_result_t coeffs;
            coeffgen_result_t fixed_coeffs;
            coeffgen_error_stats_t hw_stats;
            int sample;

            if (coeffgen_generate_segment_variant(fn, segment, variant, &coeffs) != 0) {
                return 1;
            }
            quantize_coefficients(fn, &coeffs, &fixed_coeffs);
            if (coeffgen_evaluate_segment_hw_error(fn,
                                                  segment,
                                                  &coeffs,
                                                  hardware_samples_per_segment,
                                                  &hw_stats) != 0) {
                return 1;
            }

            if (hw_stats.max_abs_error > stats->hardware.max_abs_error) {
                stats->hardware.max_abs_error = hw_stats.max_abs_error;
            }
            stats->hardware.rms_error_accum +=
                hw_stats.rms_error * hw_stats.rms_error * (long double)hw_stats.samples;
            stats->hardware.samples += hw_stats.samples;

            for (sample = 0; sample <= samples_per_segment; sample++) {
                long double h = ldexpl(1.0L, -fn->m);
                long double x = h * ((long double)sample / (long double)samples_per_segment);
                long double target = coeffgen_eval_target(fn, segment, x);
                long double approx = coeffgen_eval_polynomial(&coeffs, x);
                long double fixed_approx = coeffgen_eval_polynomial(&fixed_coeffs, x);

                accumulate_error(&stats->source, fabsl(target - approx));
                accumulate_error(&stats->fixed, fabsl(target - fixed_approx));
            }
        }
    }

    return 0;
}

static int report_variant(const char *reference_root,
                          const char *variant_root,
                          coeffgen_variant_t variant)
{
    bit_stats_t bit_stats[] = {
        {"LUTC0.txt", 29, 0, 0, 0},
        {"LUTC1.txt", 20, 0, 0, 0},
        {"LUTC2.txt", 14, 0, 0, 0},
    };
    variant_error_stats_t error_stats;
    size_t i;
    size_t j;

    for (i = 0; i < coeffgen_function_count(); i++) {
        const coeffgen_function_t *fn = coeffgen_function_at(i);
        for (j = 0; j < sizeof(bit_stats) / sizeof(bit_stats[0]); j++) {
            if (compare_lut_file(reference_root, variant_root, fn, &bit_stats[j]) != 0) {
                return 1;
            }
        }
    }

    if (compute_error_stats(variant, &error_stats) != 0) {
        return 1;
    }

    printf("variant=%s path=%s\n", coeffgen_variant_name(variant), variant_root);
    for (j = 0; j < sizeof(bit_stats) / sizeof(bit_stats[0]); j++) {
        printf("  %s rows=%ld mismatches=%ld max_unsigned_diff=%llu\n",
               bit_stats[j].name,
               bit_stats[j].rows,
               bit_stats[j].mismatches,
               bit_stats[j].max_diff);
    }
    printf("  source_error samples=%ld max_abs=%.10Le rms=%.10Le\n",
           error_stats.source.samples,
           error_stats.source.max_abs_error,
           sqrtl(error_stats.source.rms_error_accum / (long double)error_stats.source.samples));
    printf("  fixed_error  samples=%ld max_abs=%.10Le rms=%.10Le\n",
           error_stats.fixed.samples,
           error_stats.fixed.max_abs_error,
           sqrtl(error_stats.fixed.rms_error_accum / (long double)error_stats.fixed.samples));
    printf("  hw_error     samples=%ld max_abs=%.10Le rms=%.10Le\n",
           error_stats.hardware.samples,
           error_stats.hardware.max_abs_error,
           sqrtl(error_stats.hardware.rms_error_accum / (long double)error_stats.hardware.samples));

    return 0;
}

int main(int argc, char **argv)
{
    int argi;

    if (argc < 4 || ((argc - 2) % 2) != 0) {
        fprintf(stderr,
                "Usage: %s <reference_lut_dir> <variant> <variant_lut_dir> [<variant> <variant_lut_dir>...]\n",
                argv[0]);
        return 2;
    }

    for (argi = 2; argi < argc; argi += 2) {
        coeffgen_variant_t variant;

        if (coeffgen_parse_variant(argv[argi], &variant) != 0 ||
            !coeffgen_variant_available(variant)) {
            fprintf(stderr, "unsupported or unavailable variant: %s\n", argv[argi]);
            return 2;
        }

        if (report_variant(argv[1], argv[argi + 1], variant) != 0) {
            return 1;
        }
    }

    return 0;
}
