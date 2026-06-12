args = argv();

if numel(args) != 2
    fprintf(stderr, "Usage: octave --quiet run_sfu_golden_model.m <input_hex> <func>\n");
    fprintf(stderr, "Functions: 1=reci, 2=sqrt, 4=reci_sqrt, 6=exp2, 7=log2, 9=sin, 10=cos\n");
    exit(2);
endif

input_hex = args{1};
func = str2double(args{2});

if isnan(func)
    fprintf(stderr, "Invalid function selector: %s\n", args{2});
    exit(2);
endif

[result_hex, result_dec] = sfu_golden_model(input_hex, func);

printf("result_hex=%s\n", result_hex);
printf("result_dec=%.15g\n", result_dec);
