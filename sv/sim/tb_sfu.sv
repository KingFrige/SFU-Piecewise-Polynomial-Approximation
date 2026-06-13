module tb_sfu;
  logic [31:0] src1_i;
  logic [2:0] selop_i;
  logic [31:0] rro_result;
  logic [31:0] sfu_input;
  logic [31:0] result_o;
  logic quad_int_err;
  logic sel_phase;

  string data_dir;

  assign sel_phase = selop_i == 3'b100;
  assign sfu_input = (selop_i == 3'b000 || selop_i == 3'b001 || selop_i == 3'b100) ? rro_result : src1_i;

  rro u_rro (
    .selec_phase(sel_phase),
    .input_data(src1_i),
    .Result(rro_result)
  );

  sfu u_sfu (
    .src1_i(sfu_input),
    .selop_i(selop_i),
    .Result_o(result_o),
    .Quad_int_err(quad_int_err)
  );

  task automatic run_operation(
    input string name,
    input logic [2:0] selector,
    input bit include_rro_output
  );
    int in_fd;
    int out_fd;
    int scan_count;
    logic [31:0] value;
    string input_path;
    string output_path;
    string header;
    begin
      input_path = {data_dir, "/input_", name, ".csv"};
      output_path = {data_dir, "/output_", name, ".csv"};
      in_fd = $fopen(input_path, "r");
      if (in_fd == 0) begin
        $fatal(1, "failed to open %s", input_path);
      end
      out_fd = $fopen(output_path, "w");
      if (out_fd == 0) begin
        $fatal(1, "failed to open %s", output_path);
      end

      void'($fgets(header, in_fd));
      if (include_rro_output) begin
        $fwrite(out_fd, "Input,%s,RRO_output\n", name);
      end else begin
        $fwrite(out_fd, "Input,%s\n", name);
      end

      while (!$feof(in_fd)) begin
        scan_count = $fscanf(in_fd, "%h\n", value);
        if (scan_count == 1) begin
          selop_i = selector;
          src1_i = value;
          #1;
          if (include_rro_output) begin
            $fwrite(out_fd, "%08x,%08x,%08x\n", src1_i, result_o, sfu_input);
          end else begin
            $fwrite(out_fd, "%08x,%08x\n", src1_i, result_o);
          end
        end
      end

      $fclose(in_fd);
      $fclose(out_fd);
    end
  endtask

  initial begin
    if (!$value$plusargs("DATA_DIR=%s", data_dir)) begin
      data_dir = "build/SFU_Input_data";
    end

    src1_i = 32'h0;
    selop_i = 3'b000;
    #1;

    run_operation("sin", 3'b000, 1'b1);
    run_operation("cos", 3'b001, 1'b1);
    run_operation("rsqrt", 3'b010, 1'b0);
    run_operation("log2", 3'b011, 1'b0);
    run_operation("ex2", 3'b100, 1'b1);
    run_operation("rcp", 3'b101, 1'b0);
    run_operation("sqrt", 3'b110, 1'b0);
    $finish;
  end
endmodule
