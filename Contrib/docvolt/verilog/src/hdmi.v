module HDMI(
    input[7:0] OUT,
    input hdmi_serial_clk,
    input hdmi_pixel_clk,
    output tmds_clk,
    output tmds_clk_n,
    output[2:0] tmds_data,
    output[2:0] tmds_data_n
);

integer nPix = 640; //pixels per row
integer xPos = 47;
integer nLines = 480; 
integer yPos = 25;

assign HSYNC = OUT[6];
assign VSYNC = OUT[7];
wire[7:0] RED,GREEN, BLUE;
assign RED = ({OUT[1],OUT[0]} == 2'b11)? 8'hFF : 
             ({OUT[1],OUT[0]} == 2'b10)? 8'hAA :
             ({OUT[1],OUT[0]} == 2'b01)? 8'h55 : 0;
assign GREEN = ({OUT[3],OUT[2]} == 2'b11)? 8'hFF : 
               ({OUT[3],OUT[2]} == 2'b10)? 8'hAA :
               ({OUT[3],OUT[2]} == 2'b01)? 8'h55 : 0;
assign BLUE = ({OUT[5],OUT[4]} == 2'b11)? 8'hFF : 
              ({OUT[5],OUT[4]} == 2'b10)? 8'hAA :
              ({OUT[5],OUT[4]} == 2'b01)? 8'h55 : 0;

DVI_TX_Top hdmi(
		.I_rst_n(1'b1), //input I_rst_n
		.I_rgb_clk(hdmi_pixel_clk), //input I_rgb_clk
		.I_serial_clk(hdmi_serial_clk), //input I_serial_clk
		.I_rgb_vs(VSYNC), //input I_rgb_vs
		.I_rgb_hs(HSYNC), //input I_rgb_hs
		.I_rgb_de(de), //input I_rgb_de
		.I_rgb_r(RED), //input [7:0] I_rgb_r
		.I_rgb_g(GREEN), //input [7:0] I_rgb_g
		.I_rgb_b(BLUE), //input [7:0] I_rgb_b
		.O_tmds_clk_p(tmds_clk), //output O_tmds_clk_p
		.O_tmds_clk_n(tmds_clk_n), //output O_tmds_clk_n
		.O_tmds_data_p(tmds_data), //output [2:0] O_tmds_data_p
		.O_tmds_data_n(tmds_data_n) //output [2:0] O_tmds_data_n
);

//Match picture horizontally
reg de_h;
reg[9:0] pix_cnt;
always @(posedge hdmi_pixel_clk) begin
    if(HSYNC)
        pix_cnt <= pix_cnt + 1'b1;
    else  
        pix_cnt <= 0;
    
    if(pix_cnt >= xPos && pix_cnt < xPos + nPix)
        de_h <= 1;
    else 
        de_h <= 0;
end 

//Match picture vertically
reg de_v;
reg[9:0] line_cnt;
always @(posedge HSYNC) begin
    if(VSYNC)
        line_cnt <= line_cnt + 1'b1;
    else
        line_cnt <= 0;

    if(line_cnt >= yPos && line_cnt < yPos + nLines)
        de_v <= 1;
    else 
        de_v <= 0;
end

assign de = de_h && de_v;
endmodule