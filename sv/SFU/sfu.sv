module sfu (
  input  logic [31:0] src1_i,
  input  logic [2:0] selop_i,
  output logic [31:0] Result_o,
  output logic Quad_int_err
);
  localparam logic [23:0] C_HALF_PI = 24'hc90fdb;

  logic [22:0] in_mantissa;
  logic [7:0] in_exponent;
  logic in_sign;
  logic [31:0] in_sin_cos_adj;
  logic [23:0] half_pi_minus_x;
  logic [1:0] in_sin_cos_quad;
  logic [31:0] in_data_inter;
  logic [2:0] in_oper_select;
  logic [42:0] res_quad;

  logic log2_exp_eq_zero;
  logic log2_sign;
  logic log2_sign_neg;
  logic [6:0] log2_exp_sign_ext;
  logic [48:0] log2_fix_sign_ext;
  logic [6:0] log2_int_part_xor;
  logic [7:0] log2_int_part;
  logic [48:0] log2_fix_point_xor;
  logic [48:0] log2_fix_point_tmp;
  logic [48:0] log2_fix_point;
  logic [83:0] log2_mult_mx;
  logic [48:0] log2_result_unnormalized;

  logic [63:0] normalization_input;
  logic [5:0] number_of_zeros;
  logic msb_zeros;
  logic [48:0] normalized_result;
  logic rounding_even;
  logic [24:0] mantissa_rounding;
  logic [24:0] rounded_mantissa;
  logic [8:0] rounding_exponent;
  logic [8:0] normalized_exponent;
  logic [22:0] result_mantissa;

  logic signed [8:0] denormalized_input_exponent;
  logic signed [8:0] exponent_minus_one;
  logic signed [8:0] exponent_minus_one_shifted;
  logic signed [8:0] exponent_shifted;
  logic signed [8:0] rsqrt_exponent_minus_one;
  logic signed [8:0] rsqrt_exponent;
  logic signed [8:0] reciprocal_exponent;
  logic signed [8:0] final_exponent;
  logic signed [8:0] biased_final_exponent;
  logic underflow_result;

  logic result_sign_sin;
  logic result_sign_cos;
  logic result_sign_log2;
  logic result_sign_rcp;
  logic result_sign;
  logic [31:0] result_complete;

  assign in_sign = src1_i[31];
  assign in_exponent = src1_i[30:23];
  assign in_mantissa = src1_i[22:0];
  assign in_sin_cos_quad = in_exponent[7:6];

  assign half_pi_minus_x = C_HALF_PI - src1_i[23:0];
  assign in_sin_cos_adj =
    src1_i[23] ? {1'b0, 7'b0000000, half_pi_minus_x} : src1_i;
  assign in_data_inter =
    selop_i[2:1] == 2'b00 ? in_sin_cos_adj : src1_i;
  assign in_oper_select =
    selop_i[2:1] == 2'b00
      ? {2'b00, in_sin_cos_quad[0] ^ selop_i[0] ^ src1_i[23]}
      : selop_i;

  quadratic_interpolator u_quadratic_interpolator (
    .src1_i(in_data_inter),
    .selop_i(in_oper_select),
    .Res_Quad_int_o(res_quad),
    .Quad_int_err(Quad_int_err)
  );

  assign log2_exp_eq_zero = in_exponent == 8'd127;
  assign log2_mult_mx =
    res_quad[41:0] * {1'b0, in_mantissa, 18'b000000000000000000};
  assign log2_sign = in_exponent < 8'd127;
  assign log2_sign_neg = ~log2_sign;
  assign log2_exp_sign_ext = {7{log2_sign}};
  assign log2_fix_sign_ext = {49{log2_sign}};
  assign log2_int_part_xor = in_exponent[6:0] ^ log2_exp_sign_ext;
  assign log2_fix_point_xor =
    {7'b0000000, res_quad[41:0]} ^ log2_fix_sign_ext;
  assign log2_int_part =
    log2_sign_neg
      ? {1'b0, log2_int_part_xor} + 8'd1
      : {1'b0, log2_int_part_xor};
  assign log2_fix_point_tmp =
    {1'b0, log2_int_part[6:0], 41'b0} + log2_fix_point_xor;
  assign log2_fix_point =
    log2_sign ? log2_fix_point_tmp + 49'd1 : log2_fix_point_tmp;
  assign log2_result_unnormalized =
    log2_exp_eq_zero
      ? {7'b0000000, log2_mult_mx[82:41]}
      : {1'b0, log2_fix_point[47:0]};

  assign normalization_input =
    selop_i == 3'b011
      ? {log2_result_unnormalized, 15'b0}
      : {7'b0000000, res_quad[41:0], 15'b0};

  clz u_count_leading_zeros (
    .i_data(normalization_input),
    .o_zeros(number_of_zeros),
    .o_MSB_zeros(msb_zeros)
  );

  assign normalized_result =
    normalization_input[63:15] << number_of_zeros;
  assign rounding_even =
    (normalized_result[24] & normalized_result[23]) |
    (normalized_result[25] & normalized_result[24]);
  assign mantissa_rounding = {24'b0, rounding_even};
  assign rounded_mantissa =
    {1'b0, normalized_result[48:25]} + mantissa_rounding;
  assign rounding_exponent = {8'b0, rounded_mantissa[24]};
  assign normalized_exponent =
    9'd7 - {3'b000, number_of_zeros} + rounding_exponent;
  assign result_mantissa = rounded_mantissa[22:0];

  assign denormalized_input_exponent =
    $signed({1'b0, in_exponent}) - 9'sd127;
  assign exponent_minus_one = denormalized_input_exponent - 9'sd1;
  assign exponent_minus_one_shifted =
    {exponent_minus_one[8], exponent_minus_one[8:1]};
  assign exponent_shifted =
    {denormalized_input_exponent[8], denormalized_input_exponent[8:1]};
  assign rsqrt_exponent_minus_one =
    $signed(normalized_exponent) - exponent_minus_one_shifted;
  assign rsqrt_exponent =
    $signed(normalized_exponent) - exponent_shifted;
  assign reciprocal_exponent =
    $signed(normalized_exponent) - denormalized_input_exponent;

  always_comb begin
    unique case (selop_i)
      3'b000: final_exponent = $signed(normalized_exponent);
      3'b001: final_exponent = $signed(normalized_exponent);
      3'b010: begin
        final_exponent = denormalized_input_exponent[0]
          ? rsqrt_exponent_minus_one
          : rsqrt_exponent;
      end
      3'b011: final_exponent = $signed(normalized_exponent);
      3'b100: final_exponent = $signed({in_exponent[7], in_exponent});
      3'b101: final_exponent = reciprocal_exponent;
      3'b110: begin
        final_exponent = denormalized_input_exponent[0]
          ? exponent_minus_one_shifted
          : exponent_shifted;
      end
      default: final_exponent = denormalized_input_exponent;
    endcase
  end

  assign underflow_result = final_exponent < -9'sd126 || msb_zeros;
  assign biased_final_exponent =
    underflow_result ? 9'sd0 : final_exponent + 9'sd127;

  assign result_sign_sin = in_sign ^ in_sin_cos_quad[1];
  assign result_sign_cos = in_sin_cos_quad[1] ^ in_sin_cos_quad[0];
  assign result_sign_log2 = log2_sign;
  assign result_sign_rcp = in_sign;

  always_comb begin
    unique case (selop_i)
      3'b000: result_sign = result_sign_sin;
      3'b001: result_sign = result_sign_cos;
      3'b010: result_sign = 1'b0;
      3'b011: result_sign = result_sign_log2;
      3'b100: result_sign = 1'b0;
      3'b101: result_sign = result_sign_rcp;
      3'b110: result_sign = 1'b0;
      default: result_sign = src1_i[31];
    endcase
  end

  assign result_complete = {
    result_sign,
    biased_final_exponent[7:0],
    underflow_result ? 23'h000000 : result_mantissa
  };

  sfu_exceptions u_exceptions (
    .i_data_input(in_data_inter),
    .i_data_sin_cos(src1_i),
    .i_oper_result(result_complete),
    .i_selop(in_oper_select),
    .o_result_solved(Result_o)
  );
endmodule
