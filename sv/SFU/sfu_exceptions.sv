module sfu_exceptions (
  input  logic [31:0] i_data_input,
  input  logic [31:0] i_data_sin_cos,
  input  logic [31:0] i_oper_result,
  input  logic [2:0] i_selop,
  output logic [31:0] o_result_solved
);
  logic [31:0] s_sin_exeption;
  logic [31:0] s_cos_exeption;
  logic [31:0] s_rsqrt_exeption;
  logic [31:0] s_log2_exeption;
  logic [31:0] s_ex2_exeption;
  logic [31:0] s_rcp_exeption;
  logic [31:0] s_sqrt_exeption;

  always_comb begin
    if (i_data_sin_cos[30:23] == 8'hff) begin
      s_sin_exeption = 32'hffffffff;
    end else if (i_data_input[23:0] == 24'h000000) begin
      s_sin_exeption = {i_oper_result[31], 31'h00000000};
    end else begin
      s_sin_exeption = i_oper_result;
    end

    if (i_data_sin_cos[30:23] == 8'hff) begin
      s_cos_exeption = 32'hffffffff;
    end else if (i_data_input[23:0] == 24'h000000) begin
      s_cos_exeption = {i_oper_result[31], 31'h3f800000};
    end else begin
      s_cos_exeption = i_oper_result;
    end

    if (i_data_input[31]) begin
      s_rsqrt_exeption = i_data_input[30:23] == 8'h00 ? 32'hff800000 : 32'hffffffff;
    end else if (i_data_input[30:23] == 8'h00) begin
      s_rsqrt_exeption = 32'h7f800000;
    end else if (i_data_input[30:23] == 8'hff) begin
      s_rsqrt_exeption = i_data_input[22:0] == 23'h0 ? 32'h00000000 : 32'hffffffff;
    end else begin
      s_rsqrt_exeption = i_oper_result;
    end

    if (i_data_input[30:23] == 8'h00) begin
      s_log2_exeption = 32'hff800000;
    end else if (i_data_input[30:23] == 8'hff) begin
      s_log2_exeption = (!i_data_input[31] && i_data_input[22:0] == 23'h0) ? 32'h7f800000 : 32'hffffffff;
    end else if (i_data_input[31]) begin
      s_log2_exeption = 32'hffffffff;
    end else begin
      s_log2_exeption = i_oper_result;
    end

    if (i_data_input[31]) begin
      if (i_data_input[30:23] == 8'hf0 && i_data_input[22:0] == 23'h0) begin
        s_ex2_exeption = 32'h00000000;
      end else if (i_data_input[30:23] == 8'h0f && i_data_input[22:0] == 23'h0) begin
        s_ex2_exeption = 32'h7f800000;
      end else if (i_data_input[30:23] == 8'hff && i_data_input[22:0] != 23'h0) begin
        s_ex2_exeption = i_data_input;
      end else if (i_data_input[30:23] == 8'h00 && i_data_input[22:0] == 23'h0) begin
        s_ex2_exeption = 32'h3f800000;
      end else begin
        s_ex2_exeption = i_oper_result;
      end
    end else begin
      s_ex2_exeption = i_oper_result;
    end

    if (i_data_input[31]) begin
      if (i_data_input[30:23] == 8'hff && i_data_input[22:0] == 23'h0) begin
        s_rcp_exeption = 32'h80000000;
      end else if (i_data_input[30:23] == 8'hff) begin
        s_rcp_exeption = i_data_input;
      end else if (i_data_input[30:23] == 8'h00) begin
        s_rcp_exeption = 32'hff800000;
      end else begin
        s_rcp_exeption = i_oper_result;
      end
    end else if (i_data_input[30:23] == 8'hff && i_data_input[22:0] == 23'h0) begin
      s_rcp_exeption = 32'h00000000;
    end else if (i_data_input[30:23] == 8'hff) begin
      s_rcp_exeption = i_data_input;
    end else if (i_data_input[30:23] == 8'h00) begin
      s_rcp_exeption = 32'h7f800000;
    end else begin
      s_rcp_exeption = i_oper_result;
    end

    if (i_data_input[31]) begin
      if (i_data_input[30:23] == 8'h00) begin
        s_sqrt_exeption = 32'hff800000;
      end else begin
        s_sqrt_exeption = 32'hffffffff;
      end
    end else if (i_data_input[30:23] == 8'hff && i_data_input[22:0] == 23'h0) begin
      s_sqrt_exeption = 32'h7f800000;
    end else if (i_data_input[30:23] == 8'hff) begin
      s_sqrt_exeption = 32'hffffffff;
    end else if (i_data_input[30:23] == 8'h00) begin
      s_sqrt_exeption = 32'h00000000;
    end else begin
      s_sqrt_exeption = i_oper_result;
    end

    unique case (i_selop)
      3'b000: o_result_solved = s_sin_exeption;
      3'b001: o_result_solved = s_cos_exeption;
      3'b010: o_result_solved = s_rsqrt_exeption;
      3'b011: o_result_solved = s_log2_exeption;
      3'b100: o_result_solved = s_ex2_exeption;
      3'b101: o_result_solved = s_rcp_exeption;
      3'b110: o_result_solved = s_sqrt_exeption;
      default: o_result_solved = i_data_input;
    endcase
  end
endmodule
