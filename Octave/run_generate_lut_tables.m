args = argv();

if numel(args) > 0
    output_dir = args{1};
else
    output_dir = "generated_luts";
endif

if numel(args) > 1
    selectors = zeros(1, numel(args) - 1);
    for i = 2:numel(args)
        selectors(i - 1) = str2double(args{i});
        if isnan(selectors(i - 1))
            fprintf(stderr, "Invalid selector: %s\n", args{i});
            exit(2);
        endif
    endfor
else
    selectors = 1:10;
endif

summary = generate_lut_tables(output_dir, selectors);

printf("Generated %d LUT selector table set(s) in %s\n", rows(summary), output_dir);
for i = 1:rows(summary)
    printf("selector=%d name=%s rows=%d widths=%d/%d/%d m=%d\n", ...
           summary{i, 1}, summary{i, 2}, summary{i, 3}, ...
           summary{i, 4}, summary{i, 5}, summary{i, 6}, summary{i, 7});
endfor
