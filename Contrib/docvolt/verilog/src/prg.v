module PRG(
    input CLK,
    input DE, //data output enable
    input PL,
    input PH,
    input RST,
    input[7:0] Y,
    output reg[7:0] IR, //Instruction register
    output reg[7:0] D, //Data register
    inout[7:0] BUS
);
//wire D[7:0];

reg[15:0] pc = -1; //16 bit program counter
wire[31:0] dout;
wire[8:0] xadr;
wire[5:0] yadr;
wire[7:0] IR_Hi,IR_Lo;
wire[7:0] D_Hi,D_Lo;

assign BUS = DE? D : 8'bZ;

always @(posedge CLK) begin
    if(!RST)
        pc <= 0;
    else
        pc <= pc + 1'b1;
    if(PL) 
        pc[7:0] <= BUS;
    if(PH)
        pc[15:8] <= Y;
end

assign IR_Hi = dout[31:24];
assign D_Hi = dout[23:16];
assign IR_Lo = dout[15:8];
assign D_Lo = dout[7:0];

assign odd = pc[0];
wire[7:0] _IR = odd? IR_Lo : IR_Hi;
wire[7:0] _D = odd? D_Lo : D_Hi;

always @(posedge CLK) begin
    IR <= _IR;
    D <= _D;
end 

assign yadr = pc[6:1];
assign xadr = pc[15:7];

FLASH608K prg_flash (
    .DOUT(dout),
    .XE(1'b1),
    .YE(1'b1),
    .SE(CLK),
    .PROG(1'b0),
    .ERASE(1'b0),
    .NVSTR(1'b0),
    .XADR(xadr),
    .YADR(yadr),
    .DIN(32'b0)
);

endmodule



/*always @(posedge CLK)
        #1 $display("%04t prg:",$time,
            "\n  PC: %2h, IR: %2h, D: %2h, BUS: %2h", pc, IR, D, BUS, 
            "\n  PH: %b, PL: %b", PH, PL,
            "\n  xadr: %2h, yadr: %2h", xadr, yadr);
*/
