module RAM
(
    input CLK,
    input[7:0] D,
    input[7:0] X,
    input[7:0] Y,
    input EL,
    input EH,
    input OE,
    input WE,
    inout[7:0] BUS
);

//Address multiplexer
wire[14:0] addr;
assign addr[7:0] = EL? X : D;
assign addr[14:8] = EH? Y : 1'b0;

wire[7:0] dout;
reg[7:0] d = 0;
assign BUS = OE? dout : 8'bZ;

Gowin_SP ram(
    .dout(dout), //output [7:0] dout
    .clk(CLK), //input clk
    .oce(1'b1), //input oce (Invalid)
    .ce(1'b1), //input ce
    .reset(1'b0), //input reset
    .wre(WE), //input wre
    .ad(addr), //input [15:0] ad
    .din(BUS) //input [7:0] din
);
always @(negedge CLK)
    d <= dout;

endmodule



/*always @(WE, OE)
    if(WE==1 || OE==1)
         #1 $display("%04t RAM: W: %b, R: %b, [%4h]=%2h", $time, WE, OE, addr, BUS,
                "\n   D: %2x, X: %2h, Y: %2h, EH: %2h, EL: %2h", D, X, Y, EH, EL);
*/
