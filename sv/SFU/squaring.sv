module squaring #(
  parameter int word_bits = 17
) (
  input  logic [word_bits-1:0] d_in,
  output logic [(word_bits*2)-1:0] d_out
);
  always_comb begin
    d_out = d_in * d_in;
  end
endmodule
