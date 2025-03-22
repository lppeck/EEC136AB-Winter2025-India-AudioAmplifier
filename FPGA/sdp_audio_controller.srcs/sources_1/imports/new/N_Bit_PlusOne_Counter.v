`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/20/2024 12:03:28 AM
// Design Name: 
// Module Name: N_Bit_PlusOne_Counter
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


module  N_Bit_PlusOne_Counter #(parameter N = 32)(
    input wire [N-1:0] A,
    input wire b,
    output wire [N-1:0] R
    );
    
    wire [N-1:0] carry;
    
    generate
        genvar i;
        
        assign carry[0] = 1'b0;
    
        xor(R[0], b, A[0]);
        and(carry[1], b, A[0]);
        
        for(i = 1; i < N-1; i = i + 1)
        begin
            xor(R[i], carry[i], A[i]);
            and(carry[i+1], carry[i], A[i]);
        end
        
        xor(R[N-1], carry[N-1], A[N-1]);
    
    endgenerate
    
endmodule
