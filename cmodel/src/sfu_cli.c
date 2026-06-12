#include "sfu.h"

#include <stdio.h>
#include <stdlib.h>

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s <lut_dir> <input_hex> <selector>\n", argv0);
    fprintf(stderr, "Selectors: 1=reci 2=sqrt 4=reci_sqrt 6=exp2 7=log2 9=sin 10=cos\n");
}

int main(int argc, char **argv)
{
    sfu_luts_t *luts = NULL;
    uint32_t input;
    uint32_t result;
    int selector;
    char *end = NULL;
    char err[256];

    if (argc != 4) {
        usage(argv[0]);
        return 2;
    }

    if (sfu_parse_hex32(argv[2], &input) != 0) {
        fprintf(stderr, "invalid input hex: %s\n", argv[2]);
        return 2;
    }

    selector = (int)strtol(argv[3], &end, 10);
    if (end == argv[3] || *end != '\0' || !sfu_selector_supported(selector)) {
        usage(argv[0]);
        fprintf(stderr, "unsupported selector: %s\n", argv[3]);
        return 2;
    }

    if (sfu_load_luts(argv[1], &luts, err, sizeof(err)) != 0) {
        fprintf(stderr, "%s\n", err);
        return 1;
    }

    if (sfu_eval_hex(luts, input, selector, &result, err, sizeof(err)) != 0) {
        fprintf(stderr, "%s\n", err);
        sfu_free_luts(luts);
        return 1;
    }

    printf("result_hex=%08X\n", result);
    sfu_free_luts(luts);
    return 0;
}
