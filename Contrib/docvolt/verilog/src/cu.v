/*Control Unit*/
//`timescale 1ns / 1ps
// MUXes

module CU(
    input[7:0] IR,
    input CLK,
    input AC7, //Accumulator bit7
    input CO,  //ALU Carry bit
    output LD, //Load accumulator
    output OL, //Load output
    output XL, //Load X-reg
    output YL, //Load Y-Reg
    output IX, //X++ at next rising CLK
    output DE, //prg data register D->BUS
    output AE, //accumulator AC->BUS
    output PL, //Program counter LOW
    output PH, //Program conter HIGH
    output WE, //Write Ram
    output OE, //Read from RAM
    output EL, //Ram address MUX low
    output EH, //Ram address MUX hi
    output IE, //Input register => BUS
    output [2:0] AR //AR includes the AL wire (AR[4])
);

assign AR = IR[7:5];
wire store = IR[7] & IR[6] & !IR[5];
wire jump = IR[7] & IR[6] & IR[5];

//Mode
wire[7:0] mode = !jump? 1'b1 << IR[4:2] : 8'b0;
assign LD = (mode[0] | mode[1] | mode[2] | mode[3]) & !store;
assign OL = (mode[6] | mode[7]) & !store;
assign EL = (mode[1] | mode[3] | mode[7]);
assign EH = (mode[2] | mode[3] | mode[7]);
assign XL = mode[4]; 
assign YL = mode[5]; 
assign IX = mode[7];

//Bus access
wire[3:0] b_acc = 1'b1 << IR[1:0]; 
assign DE = b_acc[0]; //D -> BUS
assign OE = b_acc[1]; //RAM -> BUS
assign AE = b_acc[2]; //AC -> BUS
assign IE = b_acc[3]; //IN -> BUS

//Jump condition decoder
wire[1:0] sel = {CO, AC7};
assign pl = ((sel == 2'b00) ? IR[2] :
            (sel == 2'b01) ? IR[3] :
            (sel == 2'b10) ? IR[4] :
            1);

assign PH = jump & !IR[4] & !IR[3] & !IR[2];
assign PL = PH | jump & pl;

//Ram signals
assign WE = CLK & store;
endmodule 


/*
always @(*)
    #1 $display("%4t Control unit", $time,
       "\n IR: %2h, AR: %3b, CO: %b", IR[7:5], AR, CO);
*/

/*always @(*) 
    #1 $display ("%04t Control Unit CU", $time,
            "\n  CO: %b, AC7: %b, SEL: %d", CO, AC7, sel,
            "\n  IR: %2h, mode: %2d, PL: %b", IR, IR[4:2], PL,
            "\n");
*/

/*cond    CO  AC7
AC>0    0   0
AC<0    0   1 
AC<>0   0   x

AC=0    0   0
AC>=0   x   0
AC<=0   0   x */

