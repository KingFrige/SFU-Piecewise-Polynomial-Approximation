module clz #(
  parameter bit MODE = 1'b0
) (
  input  logic [63:0] i_data,
  output logic [5:0] o_zeros,
  output logic o_MSB_zeros
);
  logic [63:0] data;
  int count;
  int i;

  always_comb begin
    data = MODE ? ~i_data : i_data;
    count = 0;
    for (i = 63; i >= 0; i--) begin
      if (data[i] == 1'b0 && count == (63 - i)) begin
        count++;
      end
    end
    o_MSB_zeros = data == 64'h0;
    o_zeros = o_MSB_zeros ? 6'd0 : count[5:0];
  end
endmodule
