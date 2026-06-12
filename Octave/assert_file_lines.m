function assert_file_lines(path, expected_rows, expected_width)
    fid = fopen(path, "r");
    if fid < 0
        error("missing LUT file: %s", path);
    endif

    cleanup = onCleanup(@() fclose(fid));
    count = 0;

    while true
        line = fgetl(fid);
        if !ischar(line)
            break
        endif

        count += 1;
        if length(line) != expected_width
            error("%s line %d expected width %d, got %d", path, count, expected_width, length(line));
        endif

        if any(line != "0" & line != "1")
            error("%s line %d contains non-binary characters", path, count);
        endif
    endwhile

    if count != expected_rows
        error("%s expected %d rows, got %d", path, expected_rows, count);
    endif
endfunction
