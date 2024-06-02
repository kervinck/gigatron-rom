module ALU_(
    //input CLK,
    input[2:0] AR,
    input[7:0] AC,
    input[7:0] BUS,
    output [7:0] ALU,
    output CO //carry out
);

assign ALU = 
    (AR == 0)? BUS: 
    (AR == 1)? AC & BUS:
    (AR == 2)? AC | BUS:
    (AR == 3)? AC ^ BUS:
    (AR == 4)? AC + BUS:
    (AR == 5)? AC - BUS:
    (AR == 6)? AC: -AC;

assign CO = (ALU == 8'b0)? 1 : 0;

endmodule



/*    
always @(*)   
    #1 $display("%04t Arithmetic and logic unit ALU", $time,
            "\n  AR= %3b", AR,
            "\n  AC= %8b, %d",AC, AC,
            "\n  BUS=%8b, %d",BUS, BUS,
            "\n  ------------------------",
            "\n  ALU=%8b, %d, CO=%d", ALU, ALU, CO,
            "\n");
 */         
