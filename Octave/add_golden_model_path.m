function add_golden_model_path()
    persistent path_added = false;

    if path_added
        return;
    endif

    octave_dir = fileparts(mfilename("fullpath"));
    golden_model_dir = fullfile(octave_dir, "..", "Golden-model");

    if exist(golden_model_dir, "dir") != 7
        error("missing shared Golden-model directory: %s", golden_model_dir);
    endif

    addpath(golden_model_dir);
    path_added = true;
endfunction
