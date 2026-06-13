module fused_accum_tree (
  input  logic [28:0] C0,
  input  logic [19:0] C1,
  input  logic [13:0] C2,
  input  logic [15:0] X2X2,
  input  logic [16:0] X2,
  output logic [42:0] Result
);
  logic signed [63:0] c0_signed;
  logic signed [63:0] c1_signed;
  logic signed [63:0] c2_signed;
  logic signed [63:0] operation;

  always_comb begin
    c0_signed = C0[28]
      ? -$signed({36'b0, C0[27:0]})
      : $signed({36'b0, C0[27:0]});
    c1_signed = C1[19]
      ? -$signed({45'b0, C1[18:0]})
      : $signed({45'b0, C1[18:0]});
    c2_signed = C2[13]
      ? -$signed({51'b0, C2[12:0]})
      : $signed({51'b0, C2[12:0]});
    operation = (c0_signed <<< 14) +
                (c1_signed * $signed({1'b0, X2})) +
                (c2_signed * $signed({1'b0, X2X2}) * 64'sd2);
    Result = operation[42:0];
  end
endmodule
