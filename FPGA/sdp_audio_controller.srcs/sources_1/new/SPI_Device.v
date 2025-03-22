`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/07/2025 09:50:10 AM
// Design Name: 
// Module Name: SPI_Device
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


module SPI_Device #(parameter W = 16) (
    input clk,
    input cs,
    input mosi,
    output reg [W-1:0] out
    );
    
    reg [W-1:0] sr;
    and(g_clk, ~cs, clk);
    
    always@ (posedge g_clk)
    begin
        sr <= {sr[W-2:0], mosi};
    end
    
    always@ (posedge cs)
    begin
        out <= sr;
    end
endmodule
