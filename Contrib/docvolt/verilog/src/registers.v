/*There are four 8-bit registers:
    AC: The accumulator
    X,Y: Memory addresses
    OUT: Output registers */

module REGISTERS
(
    input [7:0] ALU, //Result from ALU
    input CLK2, //"CPU" clock
    input LD, //Load accumulator
    input AE, //Gate AC to BUS
    input XL, //Load Memory address, low byte
    input YL, //Load Memory address, high byte
    input IX, //Count up X
    input OL, //Load output register
    output reg[7:0] X,  //Memory addresses, Low byte, counter
    output reg[7:0] Y,  //Memory addresses High Byte
    output[7:0] BUS,//Accumulator Bus, hi-Z
    output reg[7:0] AC, //Accumulator Bus
    output reg[7:0] OUT //Periheral bus
);

initial begin
    AC = 0;
    X = 0;
    Y = 0;
end
//Accumulator
always @(posedge CLK2) 
    if(LD)
        AC <= ALU;
assign BUS = AE? AC : 8'bZ; //Bus gateway

//Output register
always @(posedge CLK2) 
    if(OL)
        OUT <= ALU;

// Program conter; addess register low byte
//reg[7:0] reg_x = {8{1'b0}};
//assign X = reg_x;
always @(posedge CLK2) begin
    if(XL)
        X <= ALU;
    else if(IX)
        X <= X + 1'b1;
end

//Address register, high byte
always @(posedge CLK2) 
    if(YL)
        Y <= ALU;

endmodule 



/*
always @(posedge CLK2)
        #1 $display("%04t registers:", $time,
            "\n  AC: %2H, OUT: %2H, X: %2H, Y: %2H", AC, OUT, X, Y,
            "\n  LD: %b OL: %b, AE: %b, XL: %b, YL: %b, IX: %b", 
            LD, OL, AE, XL, YL, IX,
            "\n");
*/            
