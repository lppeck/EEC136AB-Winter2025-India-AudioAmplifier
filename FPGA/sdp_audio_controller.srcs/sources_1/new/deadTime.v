`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/03/2025 11:26:10 AM
// Design Name: 
// Module Name: deadTime
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


module deadTime #(parameter clocks = 8)(
    input d,
    input clk,
    output a,
    output b
    );
    
    wire d_edge, counter_z;
    reg pd = 1'b0;
    reg [7:0] counter = 8'h00;
    
    xor(d_edge, pd, d);
    
    assign counter_z = (counter == 0);
    assign a = (pd & counter_z);
    assign b = (~pd & counter_z);
    
    always @(posedge clk)
    begin
        pd <= d;
        
        if(d_edge)
        begin
            counter <= clocks;
        end else begin
            if(counter_z)
            begin
                counter <= 0;
            end else begin
                counter <= counter - 1;
            end
        end
    end
    
    
    
endmodule
