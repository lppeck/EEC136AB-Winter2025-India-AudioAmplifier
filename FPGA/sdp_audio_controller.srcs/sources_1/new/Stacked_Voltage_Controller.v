`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/04/2025 09:45:30 AM
// Design Name: 
// Module Name: Stacked_Voltage_Controller
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


module Stacked_Voltage_Controller #(parameter S = 6, R = 10)(
    input [S+R-1:0] in,
    input clk,
    output reg [S-1:0] out
    );
    
    reg [R-1:0] pwm_timer_counter = 0;
    wire [S-1:0] stages_plus_one;
    
    N_Bit_PlusOne_Counter #(S) addOne (.A(in[S+R-1:R]), .b(1'b1), .R(stages_plus_one));
    
    always@ (posedge clk)
    begin
        pwm_timer_counter <= pwm_timer_counter + 1;
        
        if(pwm_timer_counter == in[R-1:0]) //in the case that in[] == 0, the output will default to low stage
        begin
            out <= in[S+R-1:R];//LOW   
        end
        else if (pwm_timer_counter == 0)
        begin
            out <= stages_plus_one;//HIGH
        end
        else
        begin
            out <= out;//NO CHANGE
        end
    end
    
endmodule
