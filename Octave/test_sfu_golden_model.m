cases = {
    "3F800000", 1,  "3F800000";
    "40800000", 2,  "40000000";
    "40800000", 4,  "3F000000";
    "3F800000", 6,  "40000000";
    "40000000", 7,  "3F800000";
    "3F490FDB", 9,  "3F3504F3";
    "C23A36C1", 10, "BF5777FD";
};

for i = 1:rows(cases)
    input_hex = cases{i, 1};
    func = cases{i, 2};
    expected_hex = cases{i, 3};

    [actual_hex, actual_dec] = sfu_golden_model(input_hex, func);

    if !strcmp(actual_hex, expected_hex)
        error("case %d failed: input=%s func=%d expected=%s actual=%s", ...
              i, input_hex, func, expected_hex, actual_hex);
    endif

    printf("ok %d input=%s func=%d result=%s dec=%.15g\n", ...
           i, input_hex, func, actual_hex, actual_dec);
endfor

printf("All %d Octave golden model regression cases passed.\n", rows(cases));
