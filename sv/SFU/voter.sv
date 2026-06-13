module voter #(
  parameter int word_bits = 2
) (
  input  logic [word_bits-1:0] z1,
  input  logic [word_bits-1:0] z2,
  input  logic [word_bits-1:0] z3,
  output logic [word_bits-1:0] z,
  output logic error
);
  logic match_12;
  logic match_23;
  logic match_13;

  generate
    if (word_bits > 1) begin : gen_vhdl_equiv
      always_comb begin
        match_12 = (z1[word_bits-1] ~^ z2[word_bits-1]) &
                   (z1[word_bits-2] ~^ z2[word_bits-2]);
        match_23 = (z2[word_bits-1] ~^ z3[word_bits-1]) &
                   (z2[word_bits-2] ~^ z3[word_bits-2]);
        match_13 = (z1[word_bits-1] ~^ z3[word_bits-1]) &
                   (z1[word_bits-2] ~^ z3[word_bits-2]);
        error = ~(match_12 | match_23 | match_13);
        z = match_13 ? z1 : z2;
      end
    end else begin : gen_one_bit
      always_comb begin
        match_12 = z1[0] ~^ z2[0];
        match_23 = z2[0] ~^ z3[0];
        match_13 = z1[0] ~^ z3[0];
        error = ~(match_12 | match_23 | match_13);
        z = match_13 ? z1 : z2;
      end
    end
  endgenerate
endmodule
