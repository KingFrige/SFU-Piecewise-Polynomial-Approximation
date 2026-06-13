module sfu_rom #(
  parameter string LUT_ROOT = "build/octave_luts"
) (
  input  logic [6:0] addr,
  input  logic [3:0] fn,
  output logic [28:0] C0,
  output logic [19:0] C1,
  output logic [13:0] C2
);
  logic [28:0] c0_reci [0:127];
  logic [28:0] c0_sqrt_1_2 [0:63];
  logic [28:0] c0_sqrt_2_4 [0:63];
  logic [28:0] c0_rsqrt_1_2 [0:127];
  logic [28:0] c0_rsqrt_2_4 [0:127];
  logic [28:0] c0_exp [0:63];
  logic [28:0] c0_ln2 [0:127];
  logic [28:0] c0_ln2e0 [0:63];
  logic [28:0] c0_sin [0:63];
  logic [28:0] c0_cos [0:63];

  logic [19:0] c1_reci [0:127];
  logic [19:0] c1_sqrt_1_2 [0:63];
  logic [19:0] c1_sqrt_2_4 [0:63];
  logic [19:0] c1_rsqrt_1_2 [0:127];
  logic [19:0] c1_rsqrt_2_4 [0:127];
  logic [19:0] c1_exp [0:63];
  logic [19:0] c1_ln2 [0:127];
  logic [19:0] c1_ln2e0 [0:63];
  logic [19:0] c1_sin [0:63];
  logic [19:0] c1_cos [0:63];

  logic [13:0] c2_reci [0:127];
  logic [13:0] c2_sqrt_1_2 [0:63];
  logic [13:0] c2_sqrt_2_4 [0:63];
  logic [13:0] c2_rsqrt_1_2 [0:127];
  logic [13:0] c2_rsqrt_2_4 [0:127];
  logic [13:0] c2_exp [0:63];
  logic [13:0] c2_ln2 [0:127];
  logic [13:0] c2_ln2e0 [0:63];
  logic [13:0] c2_sin [0:63];
  logic [13:0] c2_cos [0:63];

  initial begin
    $readmemb({LUT_ROOT, "/01_reci/LUTC0.txt"}, c0_reci);
    $readmemb({LUT_ROOT, "/02_sqrt_1_2/LUTC0.txt"}, c0_sqrt_1_2);
    $readmemb({LUT_ROOT, "/03_sqrt_2_4/LUTC0.txt"}, c0_sqrt_2_4);
    $readmemb({LUT_ROOT, "/04_reci_sqrt_1_2/LUTC0.txt"}, c0_rsqrt_1_2);
    $readmemb({LUT_ROOT, "/05_reci_sqrt_2_4/LUTC0.txt"}, c0_rsqrt_2_4);
    $readmemb({LUT_ROOT, "/06_exp/LUTC0.txt"}, c0_exp);
    $readmemb({LUT_ROOT, "/07_ln2/LUTC0.txt"}, c0_ln2);
    $readmemb({LUT_ROOT, "/08_ln2e0/LUTC0.txt"}, c0_ln2e0);
    $readmemb({LUT_ROOT, "/09_sin/LUTC0.txt"}, c0_sin);
    $readmemb({LUT_ROOT, "/10_cos/LUTC0.txt"}, c0_cos);

    $readmemb({LUT_ROOT, "/01_reci/LUTC1.txt"}, c1_reci);
    $readmemb({LUT_ROOT, "/02_sqrt_1_2/LUTC1.txt"}, c1_sqrt_1_2);
    $readmemb({LUT_ROOT, "/03_sqrt_2_4/LUTC1.txt"}, c1_sqrt_2_4);
    $readmemb({LUT_ROOT, "/04_reci_sqrt_1_2/LUTC1.txt"}, c1_rsqrt_1_2);
    $readmemb({LUT_ROOT, "/05_reci_sqrt_2_4/LUTC1.txt"}, c1_rsqrt_2_4);
    $readmemb({LUT_ROOT, "/06_exp/LUTC1.txt"}, c1_exp);
    $readmemb({LUT_ROOT, "/07_ln2/LUTC1.txt"}, c1_ln2);
    $readmemb({LUT_ROOT, "/08_ln2e0/LUTC1.txt"}, c1_ln2e0);
    $readmemb({LUT_ROOT, "/09_sin/LUTC1.txt"}, c1_sin);
    $readmemb({LUT_ROOT, "/10_cos/LUTC1.txt"}, c1_cos);

    $readmemb({LUT_ROOT, "/01_reci/LUTC2.txt"}, c2_reci);
    $readmemb({LUT_ROOT, "/02_sqrt_1_2/LUTC2.txt"}, c2_sqrt_1_2);
    $readmemb({LUT_ROOT, "/03_sqrt_2_4/LUTC2.txt"}, c2_sqrt_2_4);
    $readmemb({LUT_ROOT, "/04_reci_sqrt_1_2/LUTC2.txt"}, c2_rsqrt_1_2);
    $readmemb({LUT_ROOT, "/05_reci_sqrt_2_4/LUTC2.txt"}, c2_rsqrt_2_4);
    $readmemb({LUT_ROOT, "/06_exp/LUTC2.txt"}, c2_exp);
    $readmemb({LUT_ROOT, "/07_ln2/LUTC2.txt"}, c2_ln2);
    $readmemb({LUT_ROOT, "/08_ln2e0/LUTC2.txt"}, c2_ln2e0);
    $readmemb({LUT_ROOT, "/09_sin/LUTC2.txt"}, c2_sin);
    $readmemb({LUT_ROOT, "/10_cos/LUTC2.txt"}, c2_cos);
  end

  always_comb begin
    C0 = '0;
    C1 = '0;
    C2 = '0;
    unique case (fn)
      4'd0: begin C0 = c0_reci[addr]; C1 = c1_reci[addr]; C2 = c2_reci[addr]; end
      4'd1: begin C0 = c0_sqrt_1_2[addr[6:1]]; C1 = c1_sqrt_1_2[addr[6:1]]; C2 = c2_sqrt_1_2[addr[6:1]]; end
      4'd2: begin C0 = c0_sqrt_2_4[addr[6:1]]; C1 = c1_sqrt_2_4[addr[6:1]]; C2 = c2_sqrt_2_4[addr[6:1]]; end
      4'd3: begin C0 = c0_rsqrt_1_2[addr]; C1 = c1_rsqrt_1_2[addr]; C2 = c2_rsqrt_1_2[addr]; end
      4'd4: begin C0 = c0_rsqrt_2_4[addr]; C1 = c1_rsqrt_2_4[addr]; C2 = c2_rsqrt_2_4[addr]; end
      4'd5: begin C0 = c0_exp[addr[6:1]]; C1 = c1_exp[addr[6:1]]; C2 = c2_exp[addr[6:1]]; end
      4'd6: begin C0 = c0_ln2[addr]; C1 = c1_ln2[addr]; C2 = c2_ln2[addr]; end
      4'd7: begin C0 = c0_ln2e0[addr[6:1]]; C1 = c1_ln2e0[addr[6:1]]; C2 = c2_ln2e0[addr[6:1]]; end
      4'd8: begin C0 = c0_sin[addr[6:1]]; C1 = c1_sin[addr[6:1]]; C2 = c2_sin[addr[6:1]]; end
      4'd9: begin C0 = c0_cos[addr[6:1]]; C1 = c1_cos[addr[6:1]]; C2 = c2_cos[addr[6:1]]; end
      default: begin C0 = '0; C1 = '0; C2 = '0; end
    endcase
  end
endmodule
