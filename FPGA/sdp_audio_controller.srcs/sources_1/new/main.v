`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/07/2025 12:39:15 PM
// Design Name: 
// Module Name: main
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


module main(
    input clk100MHz,
    input spi_cs,
    input spi_clk,
    input spi_mosi,
    output [5:0] stage_H,
    output [5:0] stage_L
    );
    
    localparam S = 6; //number of stages
    localparam R = 10; //resolution of PWM signal
    localparam SPI_WORD_LENGTH = 16; 
    
    wire clk800MHz;
    wire [SPI_WORD_LENGTH-1:0] spi_output;
    wire [S-1:0] stages_control;
    
    clk_wiz_0 clock_module
    (
        .clk_out1(clk800MHz),
        .reset(1'b1),
        .locked(),
        .clk_in1(clk100MHz)
    );
    
    SPI_Device #(.W(SPI_WORD_LENGTH)) spi_module
    (
        .clk(spi_clk),
        .mosi(spi_mosi),
        .cs(spi_cs),
        .out(spi_output)
    );
    
    Stacked_Voltage_Controller #(.S(S), .R(R)) svc
    (
        .in(spi_output),
        .clk(clk800MHz),
        .out(stages_control)
    );
    
    genvar i;
    generate

        for(i = 0; i < S; i = i + 1)
        begin
            deadTime dt_module
            (
                .d(stages_control[i]),
                .clk(clk800Mhz),
                .a(stage_H[i]),
                .b(stage_L[i])
            );
        end

    endgenerate
    
endmodule
