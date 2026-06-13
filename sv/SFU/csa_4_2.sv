module csa_4_2 (
  input  logic ci,
  input  logic X1,
  input  logic X2,
  input  logic X3,
  input  logic X4,
  output logic co,
  output logic C,
  output logic S
);
  logic sxor1;
  logic sxor2;
  logic sxor3;

  always_comb begin
    sxor1 = X1 ^ X2;
    sxor2 = X3 ^ X4;
    sxor3 = sxor1 ^ sxor2;
    S = sxor3 ^ ci;
    co = (X3 & X4) | (X2 & X4) | (X2 & X3);
    C = sxor3 ? ci : X1;
  end
endmodule
