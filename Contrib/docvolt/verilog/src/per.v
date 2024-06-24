module PER(
    input[7:0] AC,
    input HSYNC,
    input VSYNC,
    input SER_DATA,
    input IE,
    input KBCLK,
    input KBDTA,
    input BTN,
    output[7:0] BUS,
    output reg[7:0] EXT_OUT
    );



reg[7:0] input_reg;
assign BUS = IE? input_reg : 8'bZ;

//assign input_reg = charbuf;
//Extended output register
always @(posedge HSYNC)
    EXT_OUT <= AC;

//Game controller input shift register
reg sd;
always @(posedge HSYNC) begin
    if(!BTN) begin
        input_reg <= 8'b11011111;
    end else begin 
        sd <= SER_DATA; 
        input_reg <= {input_reg[6:0], sd};
    end
end


/*
reg[32:0] kbdta;
reg[3:0] bitCntr;
always @(negedge KBCLK) begin
    if(!key_released)
        kbdta <= {KBDTA, kbdta[32:1]};
    else 
        kbdta <= {1'b1, kbdta[32:1]};
end

always @(posedge KBCLK)
    if(bitCntr < 10)
        bitCntr <= bitCntr + 1'b1;
    else
        bitCntr = 0;

wire[7:0] PS2Byte0 = kbdta[30:23];
wire[7:0] PS2Byte1 = kbdta[19:12];
wire[7:0] PS2Byte2 = kbdta[8:1];


reg[7:0] charbuf = 8'b11111111;
reg key_released;
//LUT for Keycodes
always @(posedge KBCLK) begin
    if(bitCntr == 10) begin
        if(PS2Byte1 == 8'hE0)
            case(PS2Byte0)
                8'h74: charbuf <= 8'b11111110; //Arrow right
                8'h6B: charbuf <= 8'b11111101; //Arrow left
                8'h72: charbuf <= 8'b11111011; //Arrow down
                8'h75: charbuf <= 8'b11110111; //Arrow up
                default: charbuf <= 8'b11111111; //Idle
            endcase
        else 
            case (PS2Byte0)
                8'h0C: charbuf <= 8'b11101111; //Start (F4)
                8'h04: charbuf <= 8'b11011111; //Select (F3)
                8'h06: charbuf <= 8'b10111111; //Button A (F1)
                8'h05: charbuf <= 8'b01111111; //Button B (F2)
                8'h5A: charbuf <= "\n";
                8'h1C: charbuf <= "a"; 
                8'h32: charbuf <= "b"; 
                8'h21: charbuf <= "c";
                8'h23: charbuf <= "d";
                8'h24: charbuf <= "e";
                default: charbuf <= 8'b11111111; //Idle
            endcase
    //end
        if(PS2Byte0 == 8'hF0)
            key_released <= 1;
        if(PS2Byte0 == 8'hFF)
            key_released <= 0;
    end
end
*/
endmodule