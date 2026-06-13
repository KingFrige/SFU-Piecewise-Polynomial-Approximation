#include "sfu.h"

#include "sfu_exceptions.h"
#include "sfu_internal.h"
#include "sfu_quadratic_interpolator.h"
#include "sfu_reconstruction.h"
#include "sfu_rro.h"

#include <math.h>

int sfu_eval_hex(const sfu_luts_t *luts,
                 uint32_t input,
                 int selector,
                 uint32_t *result,
                 char *err,
                 size_t err_size)
{
    uint32_t reduced = input;
    uint32_t mantissa;
    int active_selector;
    const sfu_rom_table_t *table;
    int64_t operation;
    double approx;
    double value;
    uint32_t packed;

    if (luts == NULL || result == NULL) {
        sfu_set_error(err, err_size, "invalid eval arguments");
        return -1;
    }
    if (!sfu_selector_supported(selector)) {
        sfu_set_error(err,
                      err_size,
                      "unsupported selector %d; supported: 1,2,4,6,7,9,10",
                      selector);
        return -1;
    }

    if (selector == 6) {
        reduced = sfu_rro_exp2(input);
    } else if (selector == 9 || selector == 10) {
        reduced = sfu_rro_trig(input);
    }

    active_selector = sfu_qi_active_selector(selector, reduced);
    table = sfu_table(luts, active_selector);
    mantissa = sfu_qi_mantissa_for(selector, input, reduced);

    if (sfu_qi_interpolate(table, mantissa, &operation) != 0) {
        sfu_set_error(err, err_size, "failed interpolation for selector %d", active_selector);
        return -1;
    }

    approx = ldexp((double)operation, -41);
    if (sfu_reconstruction_value(input, selector, active_selector, reduced, approx, &value) != 0) {
        sfu_set_error(err, err_size, "unsupported selector %d", selector);
        return -1;
    }

    packed = sfu_reconstruction_pack(value);
    if (selector == 6) {
        packed = sfu_reconstruction_adjust_exp2(reduced, packed);
    }

    *result = sfu_exceptions_apply(selector, input, reduced, packed);
    return 0;
}
