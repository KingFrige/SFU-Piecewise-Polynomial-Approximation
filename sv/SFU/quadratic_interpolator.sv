module quadratic_interpolator (
  input  logic [31:0] src1_i,
  input  logic [2:0] selop_i,
  output logic [42:0] Res_Quad_int_o,
  output logic Quad_int_err
);
  logic [22:0] in_mantiza;
  logic [7:0] in_exponent;
  logic m6;
  logic exp_zero;
  logic exp_even;
  logic [4:0] operation;
  logic [3:0] coeff_select;
  logic [6:0] x1;
  logic [16:0] x2;
  logic [33:0] x2x2_a;
  logic [33:0] x2x2_b;
  logic [33:0] x2x2_c;
  logic [33:0] x2x2;
  logic [15:0] x2x2_approx;
  logic [28:0] c0;
  logic [19:0] c1;
  logic [13:0] c2;
  logic [42:0] result_a;
  logic [42:0] result_b;
  logic [42:0] result_c;
  logic square_err;
  logic accum_err;

  assign in_exponent = src1_i[30:23];
  assign in_mantiza = src1_i[22:0];
  assign exp_zero = in_exponent == 8'd127;
  assign exp_even = in_exponent[0];
  assign m6 = ((~selop_i[1]) & (~selop_i[0])) |
              (selop_i[2] ~^ selop_i[1]) |
              (exp_zero & (~selop_i[2]) & selop_i[0]);
  assign x2 = {m6 & in_mantiza[16], in_mantiza[15:0]};
  assign x1 = in_mantiza[22:16];
  assign operation = {selop_i, exp_zero, exp_even};

  always_comb begin
    unique casez (operation)
      5'b000??: coeff_select = 4'd8;
      5'b001??: coeff_select = 4'd9;
      5'b010?1: coeff_select = 4'd3;
      5'b010?0: coeff_select = 4'd4;
      5'b0110?: coeff_select = 4'd6;
      5'b0111?: coeff_select = 4'd7;
      5'b100??: coeff_select = 4'd5;
      5'b101??: coeff_select = 4'd0;
      5'b110?1: coeff_select = 4'd1;
      5'b110?0: coeff_select = 4'd2;
      default: coeff_select = 4'd15;
    endcase
  end

  sfu_rom u_rom (
    .addr(x1),
    .fn(coeff_select),
    .C0(c0),
    .C1(c1),
    .C2(c2)
  );

  squaring u_square_1 (.d_in(x2), .d_out(x2x2_a));
  squaring u_square_2 (.d_in(x2), .d_out(x2x2_b));
  squaring u_square_3 (.d_in(x2), .d_out(x2x2_c));

  voter #(.word_bits(34)) u_square_voter (
    .z1(x2x2_a),
    .z2(x2x2_b),
    .z3(x2x2_c),
    .z(x2x2),
    .error(square_err)
  );

  assign x2x2_approx = {1'b0, x2x2[33:19]};

  fused_accum_tree u_accum_1 (.C0(c0), .C1(c1), .C2(c2), .X2X2(x2x2_approx), .X2(x2), .Result(result_a));
  fused_accum_tree u_accum_2 (.C0(c0), .C1(c1), .C2(c2), .X2X2(x2x2_approx), .X2(x2), .Result(result_b));
  fused_accum_tree u_accum_3 (.C0(c0), .C1(c1), .C2(c2), .X2X2(x2x2_approx), .X2(x2), .Result(result_c));

  voter #(.word_bits(43)) u_accum_voter (
    .z1(result_a),
    .z2(result_b),
    .z3(result_c),
    .z(Res_Quad_int_o),
    .error(accum_err)
  );

  assign Quad_int_err = square_err | accum_err;
endmodule
