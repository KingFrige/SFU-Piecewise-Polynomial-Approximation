module rro (
  input  logic selec_phase,
  input  logic [31:0] input_data,
  output logic [31:0] Result
);
  logic [31:0] rro_trig_out;
  logic [1:0] rro_trig_quadrant;
  logic [7:0] rro_trig_shift;
  logic [23:0] rro_trig_fixed_point;
  logic [31:0] rro_trig_result;
  logic [31:0] ieee_out_sin_cos;
  logic too_big_exponent;
  logic [7:0] exp2_shift;
  logic [31:0] exp2_fixed_point;
  logic [31:0] exp2_sign_ext;
  logic [31:0] exp2_complement;
  logic [31:0] exp2_adjusted;
  logic [31:0] exp2_final_result;

  rro_trig u_rro_trig (
    .input_data(input_data),
    .output_data(rro_trig_out),
    .quadrant(rro_trig_quadrant)
  );

  assign rro_trig_shift = 8'd127 - rro_trig_out[30:23];
  assign rro_trig_fixed_point =
    {1'b1, rro_trig_out[22:0]} >> rro_trig_shift;
  assign rro_trig_result = {
    input_data[31],
    rro_trig_quadrant,
    5'b00000,
    rro_trig_fixed_point
  };
  assign ieee_out_sin_cos =
    input_data[30:23] == 8'hff ? input_data : rro_trig_result;

  assign too_big_exponent = input_data[30:23] > 8'd133;
  assign exp2_shift = 8'd133 - input_data[30:23];
  assign exp2_fixed_point =
    {3'b001, input_data[22:0], 6'b000000} >> exp2_shift;
  assign exp2_sign_ext = {32{input_data[31]}};
  assign exp2_complement = exp2_fixed_point ^ exp2_sign_ext;
  assign exp2_adjusted =
    input_data[31] ? exp2_complement + 32'd1 : exp2_complement;

  always_comb begin
    if (input_data == 32'h7f800000) begin
      exp2_final_result = 32'h87800000;
    end else if (input_data == 32'hff800000) begin
      exp2_final_result = 32'hf8000000;
    end else if (input_data[30:23] == 8'hff &&
                 input_data[22:0] != 23'h000000) begin
      exp2_final_result = {1'b1, 8'hff, input_data[22:0]};
    end else if (too_big_exponent && !input_data[31]) begin
      exp2_final_result = 32'h87800000;
    end else if (too_big_exponent && input_data[31]) begin
      exp2_final_result = 32'hf8000000;
    end else if (input_data == 32'h00000000 ||
                 input_data[30:23] == 8'h00) begin
      exp2_final_result = 32'h80000000;
    end else begin
      exp2_final_result = {1'b0, exp2_adjusted[30:0]};
    end
  end

  assign Result = selec_phase ? exp2_final_result : ieee_out_sin_cos;
endmodule
