module booth_pp #(
  parameter int Data_widht = 8
) (
  input  logic x_i0,
  input  logic x_i1,
  input  logic x_i2,
  input  logic [Data_widht-1:0] Data_i,
  output logic [Data_widht-1:0] Data_o,
  output logic adj_o
);
  logic signo_booth;
  logic signo_oper;
  logic zero;
  logic two_m;
  logic [Data_widht-1:0] shift;
  logic [Data_widht-1:0] sign_extend;
  logic [Data_widht-1:0] data_comp;
  logic [Data_widht-1:0] zero_extend;
  logic [Data_widht-1:0] data_o_tmp;

  always_comb begin
    signo_booth = (x_i2 & ~x_i1) | (x_i2 & ~x_i0);
    two_m = (x_i2 & ~x_i1 & ~x_i0) | (~x_i2 & x_i1 & x_i0);
    zero = (x_i2 | x_i1 | x_i0) & (~x_i2 | ~x_i1 | ~x_i0);
    signo_oper = signo_booth ^ Data_i[Data_widht-1];
    shift = two_m ? {Data_i[Data_widht-2:0], 1'b0} :
                    {1'b0, Data_i[Data_widht-2:0]};
    sign_extend = {Data_widht{signo_oper}};
    data_comp = sign_extend ^ shift;
    zero_extend = {Data_widht{zero}};
    data_o_tmp = zero_extend & data_comp;
    Data_o = {~data_o_tmp[Data_widht-1], data_o_tmp[Data_widht-2:0]};
    adj_o = signo_oper & zero;
  end
endmodule
