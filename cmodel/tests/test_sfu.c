#include "sfu.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    const char *input_hex;
    int selector;
    const char *expected_hex;
} test_case_t;

static int test_vectors(const sfu_luts_t *luts)
{
    static const test_case_t cases[] = {
        {"3F800000", 1, "3F800000"},
        {"40800000", 2, "40000000"},
        {"40800000", 4, "3F000000"},
        {"3F800000", 6, "40000000"},
        {"40000000", 7, "3F800000"},
        {"3F490FDB", 9, "3F3504F3"},
        {"C23A36C1", 10, "BF5777FD"},
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint32_t input;
        uint32_t expected;
        uint32_t actual;
        char err[256];

        if (sfu_parse_hex32(cases[i].input_hex, &input) != 0 ||
            sfu_parse_hex32(cases[i].expected_hex, &expected) != 0) {
            fprintf(stderr, "failed to parse test vector %zu\n", i);
            return 1;
        }

        if (sfu_eval_hex(luts, input, cases[i].selector, &actual, err, sizeof(err)) != 0) {
            fprintf(stderr, "case %zu failed to evaluate: %s\n", i, err);
            return 1;
        }

        if (actual != expected) {
            fprintf(stderr,
                    "case %zu mismatch input=%s selector=%d expected=%08X actual=%08X\n",
                    i,
                    cases[i].input_hex,
                    cases[i].selector,
                    expected,
                    actual);
            return 1;
        }

        printf("ok %zu input=%s selector=%d result=%08X\n",
               i + 1,
               cases[i].input_hex,
               cases[i].selector,
               actual);
    }

    return 0;
}

static int make_dir_if_needed(const char *path)
{
    if (mkdir(path, 0777) == 0 || errno == EEXIST) {
        return 0;
    }

    fprintf(stderr, "failed to create %s: %s\n", path, strerror(errno));
    return -1;
}

static int make_path(char *out, size_t out_size, const char *root, const char *child)
{
    int n = snprintf(out, out_size, "%s/%s", root, child);

    return n < 0 || n >= (int)out_size ? -1 : 0;
}

static int write_text_file(const char *path, const char *text)
{
    FILE *fp = fopen(path, "w");

    if (fp == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    fputs(text, fp);
    if (fclose(fp) != 0) {
        fprintf(stderr, "failed to close %s: %s\n", path, strerror(errno));
        return -1;
    }

    return 0;
}

static int create_malformed_luts(const char *tmp_dir, char *root, size_t root_size)
{
    char table_dir[512];
    char path[512];

    if (make_dir_if_needed(tmp_dir) != 0 ||
        make_path(root, root_size, tmp_dir, "malformed_luts") != 0 ||
        make_dir_if_needed(root) != 0 ||
        make_path(table_dir, sizeof(table_dir), root, "01_reci") != 0 ||
        make_dir_if_needed(table_dir) != 0) {
        return -1;
    }

    if (make_path(path, sizeof(path), table_dir, "metadata.txt") != 0 ||
        write_text_file(path,
                        "selector=1\n"
                        "name=reci\n"
                        "m=7\n"
                        "rows=1\n"
                        "LUTC0_width=29\n"
                        "LUTC1_width=20\n"
                        "LUTC2_width=14\n") != 0) {
        return -1;
    }

    if (make_path(path, sizeof(path), table_dir, "LUTC0.txt") != 0 ||
        write_text_file(path, "0000000000000000000000000000\n") != 0 ||
        make_path(path, sizeof(path), table_dir, "LUTC1.txt") != 0 ||
        write_text_file(path, "00000000000000000000\n") != 0 ||
        make_path(path, sizeof(path), table_dir, "LUTC2.txt") != 0 ||
        write_text_file(path, "00000000000000\n") != 0) {
        return -1;
    }

    return 0;
}

static int test_negative_load(const char *tmp_dir)
{
    sfu_luts_t *luts = NULL;
    char err[256];
    char missing_root[512];
    char malformed_root[512];

    if (make_dir_if_needed(tmp_dir) != 0 ||
        make_path(missing_root, sizeof(missing_root), tmp_dir, "does-not-exist") != 0 ||
        create_malformed_luts(tmp_dir, malformed_root, sizeof(malformed_root)) != 0) {
        return 1;
    }

    if (sfu_load_luts(missing_root, &luts, err, sizeof(err)) == 0) {
        fprintf(stderr, "unexpectedly loaded missing LUT directory\n");
        sfu_free_luts(luts);
        return 1;
    }

    printf("ok missing LUT directory rejected: %s\n", err);

    if (sfu_load_luts(malformed_root, &luts, err, sizeof(err)) == 0) {
        fprintf(stderr, "unexpectedly loaded malformed LUT directory\n");
        sfu_free_luts(luts);
        return 1;
    }

    printf("ok malformed LUT directory rejected: %s\n", err);
    return 0;
}

static int test_unsupported_selector(const sfu_luts_t *luts)
{
    uint32_t result;
    char err[256];

    if (sfu_eval_hex(luts, 0x3f800000U, 3, &result, err, sizeof(err)) == 0) {
        fprintf(stderr, "unexpectedly accepted unsupported selector\n");
        return 1;
    }

    printf("ok unsupported selector rejected: %s\n", err);
    return 0;
}

int main(int argc, char **argv)
{
    sfu_luts_t *luts = NULL;
    char err[256];
    const char *lut_dir;
    const char *tmp_dir;
    int status = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <lut_dir> [tmp_dir]\n", argv[0]);
        return 2;
    }

    lut_dir = argv[1];
    tmp_dir = argc == 3 ? argv[2] : "build/test_tmp";
    if (sfu_load_luts(lut_dir, &luts, err, sizeof(err)) != 0) {
        fprintf(stderr, "failed to load LUTs: %s\n", err);
        return 1;
    }

    status |= test_vectors(luts);
    status |= test_unsupported_selector(luts);
    status |= test_negative_load(tmp_dir);

    sfu_free_luts(luts);
    return status == 0 ? 0 : 1;
}
