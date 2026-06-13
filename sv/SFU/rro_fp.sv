module rro_fp_mul (
  input  logic [31:0] entrada_x,
  input  logic [31:0] entrada_y,
  output logic [31:0] salida,
  output logic underflow,
  output logic overflow
);
  logic [23:0] mantissa_x;
  logic [23:0] mantissa_y;
  logic [47:0] mantissa_product;
  logic [47:0] mantissa_shifted;
  logic [22:0] result_mantissa;
  logic [9:0] exponent_x;
  logic [9:0] exponent_y;
  logic [9:0] final_exponent;
  logic [31:0] result;
  integer highest_bit;
  integer i;

  always_comb begin
    mantissa_x = {1'b1, entrada_x[22:0]};
    mantissa_y = {1'b1, entrada_y[22:0]};
    mantissa_product = mantissa_x * mantissa_y;
    exponent_x = {2'b00, entrada_x[30:23]};
    exponent_y = {2'b00, entrada_y[30:23]};
    final_exponent = exponent_x + exponent_y -
                     (mantissa_product[47] ? 10'd126 : 10'd127);

    highest_bit = 0;
    for (i = 0; i < 48; i = i + 1) begin
      if (mantissa_product[i]) begin
        highest_bit = i;
      end
    end
    mantissa_shifted = mantissa_product << (47 - highest_bit);
    result_mantissa = mantissa_shifted[46:24];

    overflow = 1'b0;
    underflow = 1'b0;
    result = 32'h00000000;
    if ($signed(final_exponent) > 10'sd255) begin
      overflow = 1'b1;
    end else if ($signed(final_exponent) < 10'sd0) begin
      underflow = 1'b1;
    end else begin
      result[30:23] = final_exponent[7:0];
      result[22:0] = result_mantissa;
    end

    if (entrada_x[30:0] == 31'h00000000 ||
        entrada_y[30:0] == 31'h00000000) begin
      salida = 32'h00000000;
      overflow = 1'b0;
      underflow = 1'b0;
    end else begin
      salida = {entrada_x[31] ^ entrada_y[31], result[30:0]};
    end
  end
endmodule

module rro_fp_add_sub (
  input  logic [31:0] FP_A,
  input  logic [31:0] FP_B,
  input  logic add_sub,
  output logic [31:0] FP_Z
);
  function automatic logic [27:0] align_fraction(
    input logic [27:0] frac,
    input logic [8:0] diff_exp
  );
    logic [27:0] temp;
    logic [27:0] shifted;
    logic [27:0] mask;
    logic sticky;
    integer amount;
    integer j;
    begin
      if (diff_exp[8:5] != 4'b0000) begin
        return 28'h0000001;
      end

      temp = frac;
      shifted = frac;
      sticky = 1'b0;
      for (j = 4; j >= 0; j = j - 1) begin
        if (diff_exp[j]) begin
          amount = 1 << j;
          mask = (28'h0000001 << amount) - 28'h0000001;
          sticky = |(temp & mask);
          shifted = temp >> amount;
        end else begin
          shifted = temp;
        end
        temp = shifted;
      end
      return {shifted[27:1], shifted[0] | sticky};
    end
  endfunction

  logic exp_a_ff;
  logic exp_b_ff;
  logic exp_a_zero;
  logic exp_b_zero;
  logic frac_a_zero;
  logic frac_b_zero;
  logic is_nan_a;
  logic is_nan_b;
  logic is_inf_a;
  logic is_inf_b;
  logic is_zero_a;
  logic is_zero_b;
  logic is_nan;
  logic is_inf;
  logic sign_a;
  logic sign_b;
  logic is_sub;
  logic [7:0] exp_a;
  logic [7:0] exp_b;
  logic [8:0] diff_exp_ab;
  logic [8:0] diff_exp_ba;
  logic [8:0] diff_exp;
  logic [27:0] effective_frac_a;
  logic [27:0] effective_frac_b;
  logic [27:0] effective_frac_b_align;
  logic [7:0] effective_exp;
  logic [27:0] add_ab;
  logic [27:0] sub_ab;
  logic [27:0] add_sub_ab;
  logic [24:0] sub_ba_equal_exp;
  logic [27:0] frac;
  logic result_sign;
  logic [27:0] frac_add_norm;
  logic [27:0] frac_sub_norm;
  logic [27:0] frac_norm;
  logic [7:0] exp_add_norm;
  logic [7:0] exp_sub_norm;
  logic [7:0] exp_norm;
  logic did_norm;
  logic is_two;
  logic sub_is_zero;
  logic underflow_sub;
  logic round_up;
  logic [22:0] frac_norm_rounded;
  logic overflow_result;
  logic underflow_result;
  integer leading_zeros;
  integer i;
  logic found_one;

  always_comb begin
    exp_a = FP_A[30:23];
    exp_b = FP_B[30:23];
    sign_a = FP_A[31];
    sign_b = add_sub ? FP_B[31] : ~FP_B[31];

    exp_a_ff = exp_a == 8'hff;
    exp_b_ff = exp_b == 8'hff;
    exp_a_zero = exp_a == 8'h00;
    exp_b_zero = exp_b == 8'h00;
    frac_a_zero = FP_A[22:0] == 23'h000000;
    frac_b_zero = FP_B[22:0] == 23'h000000;
    is_nan_a = exp_a_ff && !frac_a_zero;
    is_nan_b = exp_b_ff && !frac_b_zero;
    is_inf_a = exp_a_ff;
    is_inf_b = exp_b_ff;
    is_zero_a = exp_a_zero && frac_a_zero;
    is_zero_b = exp_b_zero && frac_b_zero;
    is_sub = sign_a ^ sign_b;
    is_nan = is_nan_a || is_nan_b || (is_inf_a && is_inf_b && is_sub);
    is_inf = (is_inf_a ^ is_inf_b) || (is_inf_a && is_inf_b && !is_sub);

    diff_exp_ab = {1'b0, exp_a} - {1'b0, exp_b};
    diff_exp_ba = {1'b0, exp_b} - {1'b0, exp_a};
    diff_exp = diff_exp_ab[8] ? diff_exp_ba : diff_exp_ab;
    if (!diff_exp_ab[8]) begin
      effective_frac_a = {2'b01, FP_A[22:0], 3'b000};
      effective_frac_b = {2'b01, FP_B[22:0], 3'b000};
      effective_exp = exp_a;
    end else begin
      effective_frac_a = {2'b01, FP_B[22:0], 3'b000};
      effective_frac_b = {2'b01, FP_A[22:0], 3'b000};
      effective_exp = exp_b;
    end

    effective_frac_b_align = align_fraction(effective_frac_b, diff_exp);
    add_ab = effective_frac_a + effective_frac_b_align;
    sub_ab = effective_frac_a - effective_frac_b_align;
    add_sub_ab = is_sub ? sub_ab : add_ab;
    sub_ba_equal_exp = {2'b01, FP_B[22:0]} - {2'b01, FP_A[22:0]};

    if (is_zero_a) begin
      frac = {2'b01, FP_B[22:0], 3'b000};
    end else if (is_zero_b) begin
      frac = {2'b01, FP_A[22:0], 3'b000};
    end else if (!sub_ba_equal_exp[24] && exp_a == exp_b && is_sub) begin
      frac = {sub_ba_equal_exp, 3'b000};
    end else begin
      frac = add_sub_ab;
    end

    if (sign_a == sign_b) begin
      result_sign = sign_a;
    end else if (diff_exp_ab[8]) begin
      result_sign = sign_b;
    end else if (!add_sub_ab[27]) begin
      result_sign = sign_a;
    end else begin
      result_sign = sign_b;
    end

    if (frac[27]) begin
      frac_add_norm = {1'b0, frac[27:2], frac[1] | frac[0]};
      did_norm = 1'b1;
    end else begin
      frac_add_norm = frac;
      did_norm = 1'b0;
    end
    is_two = &frac[26:2];
    exp_add_norm = effective_exp + {{7{1'b0}}, did_norm | is_two};

    leading_zeros = 0;
    found_one = 1'b0;
    for (i = 26; i >= 0; i = i - 1) begin
      if (!found_one) begin
        if (frac[i]) begin
          found_one = 1'b1;
        end else begin
          leading_zeros = leading_zeros + 1;
        end
      end
    end
    sub_is_zero = frac[26:0] == 27'h0000000;
    frac_sub_norm = frac << leading_zeros;
    exp_sub_norm = sub_is_zero ? 8'h00 : effective_exp - leading_zeros[7:0];
    underflow_sub = !sub_is_zero && (int'(effective_exp) > leading_zeros);

    frac_norm = is_sub ? frac_sub_norm : frac_add_norm;
    exp_norm = is_sub ? exp_sub_norm : exp_add_norm;
    round_up = (frac_norm[2] && (frac_norm[1] || frac_norm[0])) ||
               (frac_norm[3:0] == 4'b1100);
    frac_norm_rounded = frac_norm[25:3] + {{22{1'b0}}, round_up};

    overflow_result = (exp_norm == 8'hff) && !is_sub;
    underflow_result = is_sub ? !underflow_sub : 1'b0;

    if (is_nan) begin
      FP_Z = {result_sign, 8'hff, 22'h000000, 1'b1};
    end else if (is_inf || overflow_result) begin
      FP_Z = {result_sign, 8'hff, 23'h000000};
    end else if (is_zero_a || is_zero_b) begin
      FP_Z = {result_sign, effective_exp, frac[25:3]};
    end else if (underflow_result) begin
      FP_Z = {result_sign, 31'h00000000};
    end else begin
      FP_Z = {result_sign, exp_norm, frac_norm_rounded};
    end
  end
endmodule
