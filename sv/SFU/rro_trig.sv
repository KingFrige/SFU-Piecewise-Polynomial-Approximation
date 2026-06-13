module rro_trig (
  input  logic [31:0] input_data,
  output logic [31:0] output_data,
  output logic [1:0] quadrant
);
  logic [31:0] input_positive;
  logic [31:0] mult_1;
  logic [31:0] mult_2;
  logic [31:0] mult_3;
  logic [31:0] sub_1;
  logic [31:0] output_sign;
  logic [253:0] mult_1_floor;
  logic [253:0] floor_tmp;
  logic [253:0] floor_tmp_2;
  logic [253:0] floor_tmp_3;
  logic [7:0] shift_select;
  logic [31:0] floor_fp;
  logic unused_underflow_1;
  logic unused_overflow_1;
  logic unused_underflow_2;
  logic unused_overflow_2;
  logic unused_underflow_3;
  logic unused_overflow_3;

  assign input_positive = {1'b0, input_data[30:0]};

  rro_fp_mul u_mult_1 (
    .entrada_x(input_positive),
    .entrada_y(32'h3f22f983),
    .salida(mult_1),
    .underflow(unused_underflow_1),
    .overflow(unused_overflow_1)
  );

  assign mult_1_floor = {{1'b1, mult_1[22:0]}, 230'b0};
  assign shift_select = 8'd254 - mult_1[30:23];
  assign floor_tmp = mult_1_floor >> shift_select;
  assign floor_tmp_2 = {floor_tmp[253:126], 126'b0};
  assign floor_tmp_3 = floor_tmp_2 << shift_select;
  assign floor_fp = {
    1'b0,
    floor_tmp_3 != 254'b0 ? mult_1[30:23] : 8'h00,
    floor_tmp_3[252:230]
  };

  rro_fp_mul u_mult_2 (
    .entrada_x(floor_fp),
    .entrada_y(32'h3fc90000),
    .salida(mult_2),
    .underflow(unused_underflow_2),
    .overflow(unused_overflow_2)
  );

  rro_fp_mul u_mult_3 (
    .entrada_x(floor_fp),
    .entrada_y(32'h39fdaa22),
    .salida(mult_3),
    .underflow(unused_underflow_3),
    .overflow(unused_overflow_3)
  );

  rro_fp_add_sub u_sub_1 (
    .FP_A(input_positive),
    .FP_B(mult_2),
    .add_sub(1'b0),
    .FP_Z(sub_1)
  );

  rro_fp_add_sub u_sub_2 (
    .FP_A(sub_1),
    .FP_B(mult_3),
    .add_sub(1'b0),
    .FP_Z(output_sign)
  );

  assign output_data = {input_data[31], output_sign[30:0]};
  assign quadrant = floor_tmp_2[127:126];
endmodule
