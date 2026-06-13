module tb_voter;
  logic [3:0] z1;
  logic [3:0] z2;
  logic [3:0] z3;
  logic [3:0] z;
  logic error;

  voter #(.word_bits(4)) dut (
    .z1(z1),
    .z2(z2),
    .z3(z3),
    .z(z),
    .error(error)
  );

  task automatic check(input logic [3:0] a, b, c);
    logic match_12;
    logic match_23;
    logic match_13;
    logic [3:0] expected_z;
    logic expected_error;
    begin
      z1 = a;
      z2 = b;
      z3 = c;
      #1;
      match_12 = (a[3] ~^ b[3]) & (a[2] ~^ b[2]);
      match_23 = (b[3] ~^ c[3]) & (b[2] ~^ c[2]);
      match_13 = (a[3] ~^ c[3]) & (a[2] ~^ c[2]);
      expected_error = ~(match_12 | match_23 | match_13);
      expected_z = match_13 ? a : b;
      if (z !== expected_z || error !== expected_error) begin
        $fatal(1, "voter mismatch z1=%h z2=%h z3=%h expected=%h/%b actual=%h/%b",
               a, b, c, expected_z, expected_error, z, error);
      end
    end
  endtask

  initial begin
    check(4'ha, 4'ha, 4'ha);
    check(4'hc, 4'h3, 4'hc);
    check(4'h0, 4'hf, 4'h5);
    check(4'h9, 4'h8, 4'h7);
    $display("voter regression passed");
    $finish;
  end
endmodule
