#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    int width;
    unsigned long long tolerance;
    int rows;
    int mismatches;
    unsigned long long max_diff;
} lut_file_check_t;

typedef struct {
    int selector;
    const char *name;
    int rows;
} lut_dir_check_t;

static const lut_dir_check_t k_dirs[] = {
    {1, "reci", 128},
    {2, "sqrt_1_2", 64},
    {3, "sqrt_2_4", 64},
    {4, "reci_sqrt_1_2", 128},
    {5, "reci_sqrt_2_4", 128},
    {6, "exp", 64},
    {7, "ln2", 128},
    {8, "ln2e0", 64},
    {9, "sin", 64},
    {10, "cos", 64},
};

static unsigned long long parse_bits(const char *line, int width, const char *path, int row)
{
    unsigned long long value = 0;
    int i;

    for (i = 0; i < width; i++) {
        if (line[i] != '0' && line[i] != '1') {
            fprintf(stderr, "%s:%d contains non-binary data\n", path, row);
            exit(1);
        }
        value = (value << 1) | (unsigned long long)(line[i] - '0');
    }

    if (line[width] != '\0') {
        fprintf(stderr, "%s:%d expected width %d, got %zu\n", path, row, width, strlen(line));
        exit(1);
    }

    return value;
}

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

static void join_path(char *out, size_t out_size, const char *a, const char *b, const char *c)
{
    if (snprintf(out, out_size, "%s/%s/%s", a, b, c) >= (int)out_size) {
        fprintf(stderr, "path too long: %s/%s/%s\n", a, b, c);
        exit(1);
    }
}

static void format_dir(char *out, size_t out_size, const lut_dir_check_t *dir)
{
    if (snprintf(out, out_size, "%02d_%s", dir->selector, dir->name) >= (int)out_size) {
        fprintf(stderr, "directory name too long for %s\n", dir->name);
        exit(1);
    }
}

static int compare_metadata(const char *expected_root, const char *actual_root, const lut_dir_check_t *dir)
{
    char subdir[128];
    char expected_path[4096];
    char actual_path[4096];
    FILE *expected;
    FILE *actual;
    char expected_line[256];
    char actual_line[256];
    int line = 1;

    format_dir(subdir, sizeof(subdir), dir);
    join_path(expected_path, sizeof(expected_path), expected_root, subdir, "metadata.txt");
    join_path(actual_path, sizeof(actual_path), actual_root, subdir, "metadata.txt");

    expected = fopen(expected_path, "r");
    actual = fopen(actual_path, "r");
    if (expected == NULL || actual == NULL) {
        fprintf(stderr, "failed to open metadata for %s: %s\n", subdir, strerror(errno));
        if (expected != NULL) {
            fclose(expected);
        }
        if (actual != NULL) {
            fclose(actual);
        }
        return 1;
    }

    while (read_line(expected, expected_line, sizeof(expected_line))) {
        if (!read_line(actual, actual_line, sizeof(actual_line))) {
            fprintf(stderr, "%s ended before metadata line %d\n", actual_path, line);
            fclose(expected);
            fclose(actual);
            return 1;
        }
        if (strcmp(expected_line, actual_line) != 0) {
            fprintf(stderr,
                    "%s metadata line %d mismatch: expected '%s' got '%s'\n",
                    subdir,
                    line,
                    expected_line,
                    actual_line);
            fclose(expected);
            fclose(actual);
            return 1;
        }
        line++;
    }

    if (read_line(actual, actual_line, sizeof(actual_line))) {
        fprintf(stderr, "%s has extra metadata line %d\n", actual_path, line);
        fclose(expected);
        fclose(actual);
        return 1;
    }

    fclose(expected);
    fclose(actual);
    return 0;
}

static int compare_lut_file(const char *expected_root,
                            const char *actual_root,
                            const lut_dir_check_t *dir,
                            lut_file_check_t *check)
{
    char subdir[128];
    char expected_path[4096];
    char actual_path[4096];
    FILE *expected;
    FILE *actual;
    char expected_line[128];
    char actual_line[128];
    int row;

    format_dir(subdir, sizeof(subdir), dir);
    join_path(expected_path, sizeof(expected_path), expected_root, subdir, check->name);
    join_path(actual_path, sizeof(actual_path), actual_root, subdir, check->name);

    expected = fopen(expected_path, "r");
    actual = fopen(actual_path, "r");
    if (expected == NULL || actual == NULL) {
        fprintf(stderr, "failed to open LUT file pair %s/%s: %s\n", subdir, check->name, strerror(errno));
        if (expected != NULL) {
            fclose(expected);
        }
        if (actual != NULL) {
            fclose(actual);
        }
        return 1;
    }

    for (row = 1; row <= dir->rows; row++) {
        unsigned long long expected_value;
        unsigned long long actual_value;
        unsigned long long diff;

        if (!read_line(expected, expected_line, sizeof(expected_line)) ||
            !read_line(actual, actual_line, sizeof(actual_line))) {
            fprintf(stderr, "%s/%s missing row %d\n", subdir, check->name, row);
            fclose(expected);
            fclose(actual);
            return 1;
        }

        expected_value = parse_bits(expected_line, check->width, expected_path, row);
        actual_value = parse_bits(actual_line, check->width, actual_path, row);
        diff = expected_value > actual_value ? expected_value - actual_value : actual_value - expected_value;

        if (diff > check->max_diff) {
            check->max_diff = diff;
        }
        if (diff != 0) {
            check->mismatches++;
        }
        if (diff > check->tolerance) {
            fprintf(stderr,
                    "%s/%s row %d diff %llu exceeds tolerance %llu\n",
                    subdir,
                    check->name,
                    row,
                    diff,
                    check->tolerance);
            fclose(expected);
            fclose(actual);
            return 1;
        }
    }

    if (read_line(expected, expected_line, sizeof(expected_line)) ||
        read_line(actual, actual_line, sizeof(actual_line))) {
        fprintf(stderr, "%s/%s has unexpected extra rows\n", subdir, check->name);
        fclose(expected);
        fclose(actual);
        return 1;
    }

    check->rows += dir->rows;
    fclose(expected);
    fclose(actual);
    return 0;
}

int main(int argc, char **argv)
{
    lut_file_check_t files[] = {
        {"LUTC0.txt", 29, 0, 0, 0, 0},
        {"LUTC1.txt", 20, 0, 0, 0, 0},
        {"LUTC2.txt", 14, 0, 0, 0, 0},
    };
    size_t i;
    size_t j;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <expected_octave_luts_dir> <actual_c_luts_dir>\n", argv[0]);
        return 2;
    }

    for (i = 0; i < sizeof(k_dirs) / sizeof(k_dirs[0]); i++) {
        if (compare_metadata(argv[1], argv[2], &k_dirs[i]) != 0) {
            return 1;
        }

        for (j = 0; j < sizeof(files) / sizeof(files[0]); j++) {
            if (compare_lut_file(argv[1], argv[2], &k_dirs[i], &files[j]) != 0) {
                return 1;
            }
        }
    }

    for (j = 0; j < sizeof(files) / sizeof(files[0]); j++) {
        printf("%s rows=%d mismatches=%d max_unsigned_diff=%llu tolerance=%llu\n",
               files[j].name,
               files[j].rows,
               files[j].mismatches,
               files[j].max_diff,
               files[j].tolerance);
    }

    return 0;
}
