#ifndef SFU_H
#define SFU_H

#include <stddef.h>
#include <stdint.h>

typedef struct sfu_luts sfu_luts_t;

int sfu_load_luts(const char *root,
                         sfu_luts_t **out,
                         char *err,
                         size_t err_size);
void sfu_free_luts(sfu_luts_t *luts);

int sfu_eval_hex(const sfu_luts_t *luts,
                        uint32_t input,
                        int selector,
                        uint32_t *result,
                        char *err,
                        size_t err_size);

int sfu_parse_hex32(const char *text, uint32_t *value);
const char *sfu_selector_name(int selector);
int sfu_selector_supported(int selector);

#endif
