output_dir = fullfile(tempdir(), sprintf("sfu_lut_tables_%d", round(time() * 1000)));
summary = generate_lut_tables(output_dir, 1:10);

if rows(summary) != 10
    error("expected 10 selector summaries, got %d", rows(summary));
endif

expected_names = {
    "reci";
    "sqrt_1_2";
    "sqrt_2_4";
    "reci_sqrt_1_2";
    "reci_sqrt_2_4";
    "exp";
    "ln2";
    "ln2e0";
    "sin";
    "cos"
};

for i = 1:10
    func = summary{i, 1};
    name = summary{i, 2};
    row_count = summary{i, 3};
    c0_width = summary{i, 4};
    c1_width = summary{i, 5};
    c2_width = summary{i, 6};
    m = summary{i, 7};

    if func != i
        error("summary row %d has selector %d", i, func);
    endif

    if !strcmp(name, expected_names{i})
        error("selector %d expected name %s, got %s", i, expected_names{i}, name);
    endif

    if row_count != 2^m
        error("selector %d expected %d rows from m=%d, got %d", i, 2^m, m, row_count);
    endif

    if c0_width != 29 || c1_width != 20 || c2_width != 14
        error("selector %d unexpected widths %d/%d/%d", i, c0_width, c1_width, c2_width);
    endif

    func_dir = fullfile(output_dir, sprintf("%02d_%s", func, name));
    assert_file_lines(fullfile(func_dir, "LUTC0.txt"), row_count, c0_width);
    assert_file_lines(fullfile(func_dir, "LUTC1.txt"), row_count, c1_width);
    assert_file_lines(fullfile(func_dir, "LUTC2.txt"), row_count, c2_width);

    metadata_path = fullfile(func_dir, "metadata.txt");
    if exist(metadata_path, "file") != 2
        error("missing metadata file: %s", metadata_path);
    endif
endfor

summary_path = fullfile(output_dir, "summary.csv");
if exist(summary_path, "file") != 2
    error("missing summary file: %s", summary_path);
endif

printf("Generated LUT tables under %s\n", output_dir);
printf("All LUT table generation tests passed.\n");
