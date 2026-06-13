#include "sfu.h"
#include "sfu_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int make_path(char *out, size_t out_size, const char *root, const char *dir, const char *file)
{
    int n = snprintf(out, out_size, "%s/%s/%s", root, dir, file);

    return n < 0 || n >= (int)out_size ? -1 : 0;
}

static int strip_line(char *line)
{
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }

    return (int)len;
}

static int parse_binary_row(const char *line, int width, uint32_t *value)
{
    int i;
    uint32_t parsed = 0;

    if ((int)strlen(line) != width || width <= 0 || width > 31) {
        return -1;
    }

    for (i = 0; i < width; i++) {
        if (line[i] != '0' && line[i] != '1') {
            return -1;
        }
        parsed = (parsed << 1) | (uint32_t)(line[i] - '0');
    }

    *value = parsed;
    return 0;
}

static int parse_metadata(const char *path, sfu_rom_table_t *table, char *err, size_t err_size)
{
    FILE *fp;
    char line[256];

    fp = fopen(path, "r");
    if (fp == NULL) {
        sfu_set_error(err, err_size, "failed to open metadata %s", path);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char key[128];
        char value[128];

        strip_line(line);
        if (sscanf(line, "%127[^=]=%127s", key, value) != 2) {
            continue;
        }

        if (strcmp(key, "selector") == 0) {
            table->selector = atoi(value);
        } else if (strcmp(key, "name") == 0) {
            strncpy(table->name, value, sizeof(table->name) - 1);
            table->name[sizeof(table->name) - 1] = '\0';
        } else if (strcmp(key, "m") == 0) {
            table->m = atoi(value);
        } else if (strcmp(key, "rows") == 0) {
            table->rows = atoi(value);
        } else if (strcmp(key, "LUTC0_width") == 0) {
            table->width0 = atoi(value);
        } else if (strcmp(key, "LUTC1_width") == 0) {
            table->width1 = atoi(value);
        } else if (strcmp(key, "LUTC2_width") == 0) {
            table->width2 = atoi(value);
        }
    }

    fclose(fp);

    if (table->selector <= 0 || table->selector >= SFU_TABLE_COUNT ||
        table->m <= 0 || table->rows <= 0 ||
        table->width0 != SFU_C0_WIDTH ||
        table->width1 != SFU_C1_WIDTH ||
        table->width2 != SFU_C2_WIDTH) {
        sfu_set_error(err, err_size, "invalid metadata in %s", path);
        return -1;
    }

    return 0;
}

static int load_lut_file(const char *path,
                         int width,
                         int rows,
                         uint32_t **out,
                         char *err,
                         size_t err_size)
{
    FILE *fp;
    uint32_t *values;
    char line[256];
    int row = 0;

    values = (uint32_t *)calloc((size_t)rows, sizeof(*values));
    if (values == NULL) {
        sfu_set_error(err, err_size, "out of memory loading %s", path);
        return -1;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        free(values);
        sfu_set_error(err, err_size, "failed to open LUT %s", path);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        strip_line(line);
        if (row >= rows) {
            fclose(fp);
            free(values);
            sfu_set_error(err, err_size, "too many rows in %s", path);
            return -1;
        }
        if (parse_binary_row(line, width, &values[row]) != 0) {
            fclose(fp);
            free(values);
            sfu_set_error(err, err_size, "invalid row %d in %s", row + 1, path);
            return -1;
        }
        row++;
    }

    fclose(fp);

    if (row != rows) {
        free(values);
        sfu_set_error(err, err_size, "expected %d rows in %s, got %d", rows, path, row);
        return -1;
    }

    *out = values;
    return 0;
}

static int load_one_table(const char *root,
                          const sfu_op_desc_t *desc,
                          sfu_rom_table_t *table,
                          char *err,
                          size_t err_size)
{
    char dir[128];
    char path[4096];

    memset(table, 0, sizeof(*table));
    snprintf(dir, sizeof(dir), "%02d_%s", desc->selector, desc->name);

    if (make_path(path, sizeof(path), root, dir, "metadata.txt") != 0 ||
        parse_metadata(path, table, err, err_size) != 0) {
        return -1;
    }

    if (table->selector != desc->selector || strcmp(table->name, desc->name) != 0) {
        sfu_set_error(err, err_size, "metadata mismatch in %s", path);
        return -1;
    }

    if (make_path(path, sizeof(path), root, dir, "LUTC0.txt") != 0 ||
        load_lut_file(path, table->width0, table->rows, &table->c0, err, err_size) != 0) {
        return -1;
    }
    if (make_path(path, sizeof(path), root, dir, "LUTC1.txt") != 0 ||
        load_lut_file(path, table->width1, table->rows, &table->c1, err, err_size) != 0) {
        return -1;
    }
    if (make_path(path, sizeof(path), root, dir, "LUTC2.txt") != 0 ||
        load_lut_file(path, table->width2, table->rows, &table->c2, err, err_size) != 0) {
        return -1;
    }

    return 0;
}

static sfu_luts_t *alloc_luts(char *err, size_t err_size)
{
    sfu_luts_t *luts = (sfu_luts_t *)calloc(1, sizeof(*luts));

    if (luts == NULL) {
        sfu_set_error(err, err_size, "out of memory");
    }

    return luts;
}

void sfu_free_luts(sfu_luts_t *luts)
{
    int i;

    if (luts == NULL) {
        return;
    }

    for (i = 0; i < SFU_TABLE_COUNT; i++) {
        free(luts->table[i].c0);
        free(luts->table[i].c1);
        free(luts->table[i].c2);
    }
    free(luts);
}

int sfu_load_luts(const char *root, sfu_luts_t **out, char *err, size_t err_size)
{
    sfu_luts_t *luts;
    const sfu_op_desc_t *ops;
    size_t count;
    size_t i;

    if (root == NULL || out == NULL) {
        sfu_set_error(err, err_size, "invalid LUT load arguments");
        return -1;
    }

    luts = alloc_luts(err, err_size);
    if (luts == NULL) {
        return -1;
    }

    ops = sfu_ops_all(&count);
    for (i = 0; i < count; i++) {
        const int selector = ops[i].selector;
        sfu_rom_table_t *table = sfu_mutable_table(luts, selector);

        if (table == NULL || load_one_table(root, &ops[i], table, err, err_size) != 0) {
            sfu_free_luts(luts);
            return -1;
        }
    }

    *out = luts;
    return 0;
}
