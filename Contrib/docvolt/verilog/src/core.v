module CORE
(
    input CLK,
    output DE,
    output PL,
    output PH,
    output OE,
    output IE,
    output WE,
    output EL,
    output EH,
    input[7:0] IR,
    input[7:0] D,
    inout[7:0] BUS, //Accumulator Bus
    output[7:0] X,  
    output[7:0] Y,  
    output[7:0] OUT,
    output[7:0] AC
);

wire LD; //Load accumulator
wire YL; //Load Memory address, high byte
wire XL; //Load Memory address, low byte
wire OL; //Load output register
wire IX; //Count up X
wire AE; //Gate AC to BUS
   
wire[7:0] ALU; //ALU Results bus


wire[2:0] AR; //ALU control
wire CO; //Indicates AC = 0

CU control( //Control unit
    .CLK(CLK),
    .IR(IR),
    .AC7(AC[7]),
    .CO(CO),
    .LD(LD),
    .OL(OL),
    .XL(XL),
    .YL(YL),
    .IX(IX),
    .DE(DE),
    .AE(AE),
    .PL(PL),
    .PH(PH),
    .WE(WE),
    .OE(OE),
    .EL(EL),
    .EH(EH),
    .IE(IE),
    .AR(AR)
);

ALU_ alu( //Arithmetic and logic unit
    .AR(AR),
    .AC(AC),
    .BUS(BUS),
    .ALU(ALU),
    .CO(CO)
);

REGISTERS registers( //User registers
    .ALU(ALU),
    .CLK2(!CLK),
    .LD(LD),
    .AE(AE),
    .XL(XL),
    .YL(YL),
    .IX(IX),
    .OL(OL),
    .X(X),
    .Y(Y),
    .BUS(BUS),
    .AC(AC),
    .OUT(OUT)
);

endmodule