`timescale 1ps / 1fs
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/03/2025 06:49:17 PM
// Design Name: 
// Module Name: deadTime_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module deadTime_tb;

 localparam S = 6; //number of stages
 localparam R = 10; //resolution of PWM signal
 localparam SPI_WORD_LENGTH = 16; 

reg clk = 0;;

always #625 clk = ~clk;

reg [5:0] i = 0; //3 stages, 3bit pwm res
wire [5:0] j;
assign j = i + 8;

reg [S+R-1:0] audio_sample = 0;

wire [S-1:0] stages_control, stage_L, stage_H;



//deadTime #(.clocks(10)) dtu (.d(D), .clk(clk), .a(a), .b(b));
    
Stacked_Voltage_Controller #(.S(S), .R(R)) svc_test
    (
        .in(audio_sample),
        .clk(clk),
        .out(stages_control)
    );
    
    
    
        genvar x;
    generate

        for(x = 0; x < S; x = x + 1)
        begin
            deadTime dt_module
            (
                .d(stages_control[x]),
                .clk(clk),
                .a(stage_H[x]),
                .b(stage_L[x])
            );
        end

    endgenerate

initial begin
    #10000000 audio_sample = 16'h5F39;
    #10000000 audio_sample = 16'h02D3;  
    #10000000 audio_sample = 16'h82C4;  
    #10000000 audio_sample = 16'h0002;
end


endmodule
