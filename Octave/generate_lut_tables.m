function summary = generate_lut_tables(output_dir, selectors)
    add_golden_model_path();

    if nargin < 1 || isempty(output_dir)
        output_dir = "generated_luts";
    endif

    if nargin < 2 || isempty(selectors)
        selectors = 1:10;
    endif

    if isstring(output_dir)
        output_dir = char(output_dir);
    endif

    if exist(output_dir, "dir") != 7
        mkdir(output_dir);
    endif

    summary = {};

    for i = 1:numel(selectors)
        func = selectors(i);
        [LUTC0, LUTC1, LUTC2, m] = loadLUTs(func);
        name = lut_selector_name(func);
        func_dir = fullfile(output_dir, sprintf("%02d_%s", func, name));

        if exist(func_dir, "dir") != 7
            mkdir(func_dir);
        endif

        write_lut_matrix(fullfile(func_dir, "LUTC0.txt"), LUTC0);
        write_lut_matrix(fullfile(func_dir, "LUTC1.txt"), LUTC1);
        write_lut_matrix(fullfile(func_dir, "LUTC2.txt"), LUTC2);
        write_lut_metadata(fullfile(func_dir, "metadata.txt"), func, name, m, LUTC0, LUTC1, LUTC2);

        summary(end + 1, :) = {func, name, rows(LUTC0), columns(LUTC0), columns(LUTC1), columns(LUTC2), m};
    endfor

    write_summary(fullfile(output_dir, "summary.csv"), summary);
endfunction

function name = lut_selector_name(func)
    names = {
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

    if func < 1 || func > rows(names)
        error("Unsupported LUT selector: %d", func);
    endif

    name = names{func};
endfunction

function write_lut_matrix(path, matrix)
    fid = fopen(path, "w");
    if fid < 0
        error("Unable to open %s for writing", path);
    endif

    cleanup = onCleanup(@() fclose(fid));

    for row = 1:rows(matrix)
        fprintf(fid, "%s\n", matrix(row, :));
    endfor
endfunction

function write_lut_metadata(path, func, name, m, LUTC0, LUTC1, LUTC2)
    fid = fopen(path, "w");
    if fid < 0
        error("Unable to open %s for writing", path);
    endif

    cleanup = onCleanup(@() fclose(fid));

    fprintf(fid, "selector=%d\n", func);
    fprintf(fid, "name=%s\n", name);
    fprintf(fid, "m=%d\n", m);
    fprintf(fid, "rows=%d\n", rows(LUTC0));
    fprintf(fid, "LUTC0_width=%d\n", columns(LUTC0));
    fprintf(fid, "LUTC1_width=%d\n", columns(LUTC1));
    fprintf(fid, "LUTC2_width=%d\n", columns(LUTC2));
endfunction

function write_summary(path, summary)
    fid = fopen(path, "w");
    if fid < 0
        error("Unable to open %s for writing", path);
    endif

    cleanup = onCleanup(@() fclose(fid));

    fprintf(fid, "selector,name,rows,LUTC0_width,LUTC1_width,LUTC2_width,m\n");
    for i = 1:rows(summary)
        fprintf(fid, "%d,%s,%d,%d,%d,%d,%d\n", ...
                summary{i, 1}, summary{i, 2}, summary{i, 3}, ...
                summary{i, 4}, summary{i, 5}, summary{i, 6}, summary{i, 7});
    endfor
endfunction
