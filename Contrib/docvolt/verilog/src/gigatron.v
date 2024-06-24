module GIGATRON(
    input btn1,
    input btn2,
    input sys_clk,  // Builtin 27MHz crystal
    input ser_data, // Game console input
    input kbclk,    // PS/2 keyboard clock (green)
    input kbdta,   // PS/2 keyboard data (white)
    output[5:0] led,
    output[7:0] out,
    output tmds_clk,
    output tmds_clk_n,
    output[2:0] tmds_data,
    output[2:0] tmds_data_n,
    output[3:0] audio
);

//User Registers
wire DE;    //Data register =< BUS
wire[7:0] AC; //Accumulator
wire[7:0] X;  //Memory address register, Low byte, counter
wire[7:0] Y;  //Memory address register High Byte
wire[7:0] BUS; //Bus
wire[7:0] OUT; //Output register

//Instruction fetch
wire PL;    //Set PC Low
wire PH;    //Set PC Hi
wire[7:0] IR; //Instruction bus
wire[7:0] D;  //Program data

//Ram signals
wire OE; //Read from RAM
wire IE; //Read from Input
wire WE; //Write to RAM
wire EL; //Ram address selector
wire EH; //Ram address selector

//IO Registers 
wire[7:0] EXT_OUT;
assign led = ~EXT_OUT[3:0];
assign audio = EXT_OUT[7:4];
assign out = OUT;

CORE core(
    .CLK(core_clk),
    .DE(DE),
    .PL(PL),
    .PH(PH),
    .X(X),
    .Y(Y),
    .IR(IR),
    .D(D),
    .EL(EL),
    .EH(EH),
    .OE(OE),
    .IE(IE),
    .WE(WE),
    .AC(AC),
    .BUS(BUS),
    .OUT(OUT)
);

//Instruction fetch
PRG prg(
    .CLK(core_clk),
    .RST(btn1),
    .DE(DE),
    .PL(PL),
    .PH(PH),
    .Y(Y),
    .IR(IR),
    .D(D),
    .BUS(BUS)
);

//Memory address unit
RAM ram(
    .CLK(hdmi_pixel_clk),
    .D(D),
    .X(X),
    .Y(Y),
    .EL(EL),
    .EH(EH),
    .OE(OE),
    .WE(WE),
    .BUS(BUS)
);

//Peripherals
PER per(
    .AC(AC),
    .IE(IE),
    .SER_DATA(ser_data),
    .KBCLK(kbclk),
    .KBDTA(kbdta),
    .BTN(btn2),
    .HSYNC(OUT[6]),
    .VSYNC(OUT[7]),
    .BUS(BUS),
    .EXT_OUT(EXT_OUT)
);

HDMI hdmi(
    .OUT(OUT),
    .hdmi_serial_clk(hdmi_serial_clk),
    .hdmi_pixel_clk(hdmi_pixel_clk),
    .tmds_clk(tmds_clk),
    .tmds_clk_n(tmds_clk_n),
    .tmds_data(tmds_data),
    .tmds_data_n(tmds_data_n)
);

//Clocks
Gowin_rPLL pll( //130 MHz
        .clkout(hdmi_serial_clk), //output clkout
        .clkin(sys_clk) //input clkin
);
/*OSC osc_master (
    .OSCOUT(hdmi_serial_clk) //~125 MHz
);*/
defparam osc_master.FREQ_DIV = 2;
defparam osc_master.DEVICE = "GW1NR-9C";

CLKDIV div5 (
    .CLKOUT(hdmi_pixel_clk), //26MHz
    .HCLKIN(hdmi_serial_clk), //130MHz
    .RESETN(1'b1),
    .CALIB(1'b0)
);
defparam div5.DIV_MODE = "5";
defparam div5.GSREN = "false";

CLKDIV div4 (
    .CLKOUT(core_clk), //6.5MHz
    .HCLKIN(hdmi_pixel_clk), //26MHz
    .RESETN(1'b1),
    .CALIB(1'b0)
);
defparam div4.DIV_MODE = "4";
defparam div4.GSREN = "false";

endmodule
