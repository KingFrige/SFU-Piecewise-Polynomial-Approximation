#ifndef SFU_INTERNAL_H
#define SFU_INTERNAL_H

#include "sfu.h"

#define SFU_TABLE_COUNT 11
#define SFU_C0_WIDTH 29
#define SFU_C1_WIDTH 20
#define SFU_C2_WIDTH 14

typedef struct {
    int selector;
    const char *name;
} sfu_op_desc_t;

typedef struct {
    int selector;
    char name[64];
    int m;
    int rows;
    int width0;
    int width1;
    int width2;
    uint32_t *c0;
    uint32_t *c1;
    uint32_t *c2;
} sfu_rom_table_t;

struct sfu_luts {
    sfu_rom_table_t table[SFU_TABLE_COUNT];
};

void sfu_set_error(char *err, size_t err_size, const char *fmt, ...);
const sfu_op_desc_t *sfu_ops_all(size_t *count);

static inline const sfu_rom_table_t *sfu_table(const sfu_luts_t *luts, int selector)
{
    if (luts == NULL || selector <= 0 || selector >= SFU_TABLE_COUNT) {
        return NULL;
    }

    return &luts->table[selector];
}

static inline sfu_rom_table_t *sfu_mutable_table(sfu_luts_t *luts, int selector)
{
    if (luts == NULL || selector <= 0 || selector >= SFU_TABLE_COUNT) {
        return NULL;
    }

    return &luts->table[selector];
}

#endif
