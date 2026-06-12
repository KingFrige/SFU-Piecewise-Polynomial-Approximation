#include "coeffgen.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void print_usage(const char *argv0)
{
    size_t i;

    fprintf(stderr, "Usage: %s [--variant=name] <output_dir> [selector|name|all]\n", argv0);
    fprintf(stderr, "If selector/name is omitted, all coefficient files are generated.\n");
    fprintf(stderr, "Known variants:\n");
    for (i = 0; i < coeffgen_variant_count(); i++) {
        coeffgen_variant_t variant = coeffgen_variant_at(i);
        fprintf(stderr,
                "  %-10s %s\n",
                coeffgen_variant_name(variant),
                coeffgen_variant_available(variant) ? "" : "(not available in this build)");
    }
    fprintf(stderr, "Known selectors:\n");
    for (i = 0; i < coeffgen_function_count(); i++) {
        const coeffgen_function_t *fn = coeffgen_function_at(i);
        fprintf(stderr, "  %2d  %s\n", fn->selector, fn->name);
    }
}

static int ensure_output_dir(const char *path)
{
    if (mkdir(path, 0775) == 0) {
        return 0;
    }

    if (errno == EEXIST) {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            return 0;
        }
    }

    fprintf(stderr, "failed to create output directory %s: %s\n", path, strerror(errno));
    return 1;
}

enum {
    LUTC0_BITS = 29,
    LUTC1_BITS = 20,
    LUTC2_BITS = 14
};

static int coefficient_frac_bits(const coeffgen_function_t *fn, int coefficient)
{
    if (coefficient == 0) {
        return fn->t;
    }
    if (coefficient == 1) {
        return fn->p;
    }
    return fn->q;
}

static int coefficient_bus_bits(int coefficient)
{
    if (coefficient == 0) {
        return LUTC0_BITS;
    }
    if (coefficient == 1) {
        return LUTC1_BITS;
    }
    return LUTC2_BITS;
}

static int coefficient_tail_zeros(const coeffgen_function_t *fn, int coefficient)
{
    if (coefficient == 0) {
        return LUTC0_BITS - 2 - fn->t;
    }
    if (coefficient == 1) {
        return LUTC1_BITS - 2 - fn->p;
    }
    return LUTC2_BITS - 1 - fn->q;
}

static long double coefficient_value(const coeffgen_result_t *row, int coefficient)
{
    if (coefficient == 0) {
        return row->c0;
    }
    if (coefficient == 1) {
        return row->c1;
    }
    return row->c2;
}

static int format_abs_bits(long double value, int frac_bits, char *out, size_t out_size)
{
    int i;
    long double abs_value = value < 0.0L ? -value : value;
    long double int_part_ld = floorl(abs_value);
    int int_part = (int)int_part_ld;
    long double frac = abs_value - int_part_ld;

    if (out_size < (size_t)frac_bits + 2) {
        return 1;
    }

    out[0] = int_part ? '1' : '0';
    for (i = 0; i < frac_bits; i++) {
        frac *= 2.0L;
        if (frac >= 1.0L) {
            out[i + 1] = '1';
            frac -= 1.0L;
        } else {
            out[i + 1] = '0';
        }
    }
    out[frac_bits + 1] = '\0';
    return 0;
}

static int column_is_common(char **bits, int rows, int column)
{
    int row;
    char first = bits[0][column];

    for (row = 1; row < rows; row++) {
        if (bits[row][column] != first) {
            return 0;
        }
    }

    return 1;
}

static void remove_column(char **bits, int rows, int *bit_len, int column)
{
    int row;

    for (row = 0; row < rows; row++) {
        memmove(&bits[row][column], &bits[row][column + 1], (size_t)(*bit_len - column));
    }
    (*bit_len)--;
}

static void trim_leading_zero_concat(char *concat)
{
    char *first_one = strchr(concat, '1');

    if (first_one == NULL) {
        concat[0] = '\0';
        return;
    }

    if (first_one != concat) {
        memmove(concat, first_one, strlen(first_one) + 1);
    }
}

static int compress_lut_bits(char **bits, int rows, int *bit_len, char *concat, size_t concat_size)
{
    size_t concat_len = 0;

    concat[0] = '\0';
    while (*bit_len > 0) {
        int column;
        int found = 0;

        for (column = 0; column < *bit_len; column++) {
            if (column_is_common(bits, rows, column)) {
                int i;

                if (concat_len + (size_t)column + 2 > concat_size) {
                    return 1;
                }
                for (i = 0; i <= column; i++) {
                    concat[concat_len++] = bits[0][i];
                }
                concat[concat_len] = '\0';
                remove_column(bits, rows, bit_len, column);
                found = 1;
                break;
            }
        }

        if (!found) {
            break;
        }
    }

    trim_leading_zero_concat(concat);
    return 0;
}

static int write_binary_lut_vector(const char *path,
                                   const coeffgen_function_t *fn,
                                   const coeffgen_result_t *rows_data,
                                   int rows,
                                   int coefficient)
{
    FILE *fp = fopen(path, "w");
    char **bits;
    char concat[128];
    int frac_bits = coefficient_frac_bits(fn, coefficient);
    int bus_bits = coefficient_bus_bits(coefficient);
    int tail_zeros = coefficient_tail_zeros(fn, coefficient);
    int bit_len = frac_bits + 1;
    int sign;
    int prefix_zeros;
    int segment;

    if (fp == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return 1;
    }

    if (tail_zeros < 0 || rows < 2) {
        fprintf(stderr, "invalid LUT dimensions for %s\n", fn->name);
        fclose(fp);
        return 1;
    }

    bits = calloc((size_t)rows, sizeof(bits[0]));
    if (bits == NULL) {
        fprintf(stderr, "failed to allocate LUT bits for %s\n", fn->name);
        fclose(fp);
        return 1;
    }

    for (segment = 0; segment < rows; segment++) {
        bits[segment] = calloc((size_t)frac_bits + 2, sizeof(bits[segment][0]));
        if (bits[segment] == NULL ||
            format_abs_bits(coefficient_value(&rows_data[segment], coefficient),
                            frac_bits,
                            bits[segment],
                            (size_t)frac_bits + 2) != 0) {
            int row;
            fprintf(stderr, "failed to encode LUT bits for %s segment %d\n", fn->name, segment);
            for (row = 0; row <= segment; row++) {
                free(bits[row]);
            }
            free(bits);
            fclose(fp);
            return 1;
        }
    }

    if (compress_lut_bits(bits, rows, &bit_len, concat, sizeof(concat)) != 0) {
        int row;
        fprintf(stderr, "failed to compress LUT bits for %s\n", fn->name);
        for (row = 0; row < rows; row++) {
            free(bits[row]);
        }
        free(bits);
        fclose(fp);
        return 1;
    }

    sign = coefficient_value(&rows_data[1], coefficient) > 0.0L ? 0 : 1;
    prefix_zeros = bus_bits - 1 - (int)strlen(concat) - bit_len - tail_zeros;
    if (prefix_zeros < 0) {
        int row;
        fprintf(stderr, "encoded LUT row is wider than bus for %s\n", fn->name);
        for (row = 0; row < rows; row++) {
            free(bits[row]);
        }
        free(bits);
        fclose(fp);
        return 1;
    }

    for (segment = 0; segment < rows; segment++) {
        int i;

        fputc(sign ? '1' : '0', fp);
        for (i = 0; i < prefix_zeros; i++) {
            fputc('0', fp);
        }
        fputs(concat, fp);
        fputs(bits[segment], fp);
        for (i = 0; i < tail_zeros; i++) {
            fputc('0', fp);
        }
        fputc('\n', fp);

        if (fn->selector == 9 && coefficient == 0 && segment == 0) {
            fflush(fp);
        }
    }

    for (segment = 0; segment < rows; segment++) {
        free(bits[segment]);
    }
    free(bits);
    fclose(fp);

    if (fn->selector == 9 && coefficient == 0) {
        FILE *patch_fp = fopen(path, "r+");
        if (patch_fp == NULL) {
            fprintf(stderr, "failed to patch sine C0 sign in %s: %s\n", path, strerror(errno));
            return 1;
        }
        fputc('1', patch_fp);
        fclose(patch_fp);
    }

    return 0;
}

static int write_metadata_file(const char *path,
                               const coeffgen_function_t *fn,
                               int rows,
                               coeffgen_variant_t variant)
{
    FILE *fp = fopen(path, "w");

    if (fp == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return 1;
    }

    fprintf(fp, "selector=%d\n", fn->selector);
    fprintf(fp, "name=%s\n", fn->name);
    fprintf(fp, "m=%d\n", fn->m);
    fprintf(fp, "rows=%d\n", rows);
    fprintf(fp, "LUTC0_width=%d\n", LUTC0_BITS);
    fprintf(fp, "LUTC1_width=%d\n", LUTC1_BITS);
    fprintf(fp, "LUTC2_width=%d\n", LUTC2_BITS);
    if (variant != COEFFGEN_VARIANT_REMEZ_COMPAT) {
        fprintf(fp, "variant=%s\n", coeffgen_variant_name(variant));
    }

    fclose(fp);
    return 0;
}

static int make_child_path(char *path, size_t path_size, const char *dir, const char *name)
{
    if (snprintf(path, path_size, "%s/%s", dir, name) >= (int)path_size) {
        fprintf(stderr, "output path is too long: %s/%s\n", dir, name);
        return 1;
    }

    return 0;
}

static int write_coeff_directory(const char *output_dir,
                                 const coeffgen_function_t *fn,
                                 coeffgen_variant_t variant)
{
    char dir_path[4096];
    char path[4096];
    coeffgen_result_t *rows_data;
    int rows = coeffgen_segment_count(fn);
    int segment;

    if (rows <= 0) {
        return 1;
    }

    if (snprintf(dir_path, sizeof(dir_path), "%s/%02d_%s", output_dir, fn->selector, fn->name) >=
        (int)sizeof(dir_path)) {
        fprintf(stderr, "output directory path is too long for %s\n", fn->name);
        return 1;
    }

    if (ensure_output_dir(dir_path) != 0) {
        return 1;
    }

    rows_data = calloc((size_t)rows, sizeof(rows_data[0]));
    if (rows_data == NULL) {
        fprintf(stderr, "failed to allocate coefficient rows for %s\n", fn->name);
        return 1;
    }

    for (segment = 0; segment < rows; segment++) {
        if (coeffgen_generate_segment_variant(fn, segment, variant, &rows_data[segment]) != 0) {
            fprintf(stderr, "failed to generate %s segment %d\n", fn->name, segment);
            free(rows_data);
            return 1;
        }
    }

    if (make_child_path(path, sizeof(path), dir_path, "LUTC0.txt") != 0 ||
        write_binary_lut_vector(path, fn, rows_data, rows, 0) != 0) {
        free(rows_data);
        return 1;
    }

    if (make_child_path(path, sizeof(path), dir_path, "LUTC1.txt") != 0 ||
        write_binary_lut_vector(path, fn, rows_data, rows, 1) != 0) {
        free(rows_data);
        return 1;
    }

    if (make_child_path(path, sizeof(path), dir_path, "LUTC2.txt") != 0 ||
        write_binary_lut_vector(path, fn, rows_data, rows, 2) != 0) {
        free(rows_data);
        return 1;
    }

    if (make_child_path(path, sizeof(path), dir_path, "metadata.txt") != 0 ||
        write_metadata_file(path, fn, rows, variant) != 0) {
        free(rows_data);
        return 1;
    }

    free(rows_data);
    return 0;
}

static int write_all_functions(const char *output_dir, coeffgen_variant_t variant)
{
    size_t i;

    for (i = 0; i < coeffgen_function_count(); i++) {
        if (write_coeff_directory(output_dir, coeffgen_function_at(i), variant) != 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    const char *output_dir;
    const char *selector;
    coeffgen_variant_t variant = COEFFGEN_VARIANT_REMEZ_COMPAT;
    int argi = 1;

    if (argi < argc && strncmp(argv[argi], "--variant=", 10) == 0) {
        const char *variant_name = argv[argi] + 10;
        if (coeffgen_parse_variant(variant_name, &variant) != 0 ||
            !coeffgen_variant_available(variant)) {
            fprintf(stderr, "unsupported or unavailable variant: %s\n", variant_name);
            print_usage(argv[0]);
            return 2;
        }
        argi++;
    }

    if (argc - argi != 1 && argc - argi != 2) {
        print_usage(argv[0]);
        return 2;
    }

    output_dir = argv[argi];
    selector = argc - argi == 2 ? argv[argi + 1] : "all";

    if (ensure_output_dir(output_dir) != 0) {
        return 1;
    }

    if (strcmp(selector, "all") == 0) {
        return write_all_functions(output_dir, variant);
    }

    {
        const coeffgen_function_t *fn = coeffgen_find_function(selector);
        if (fn == NULL) {
            print_usage(argv[0]);
            return 2;
        }

        return write_coeff_directory(output_dir, fn, variant);
    }
}
