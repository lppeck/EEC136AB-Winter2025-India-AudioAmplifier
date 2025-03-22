`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/19/2024 01:14:10 PM
// Design Name: 
// Module Name: full_adder
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


module N_Bit_Adder #(parameter N = 32)(
        input wire [N-1:0] A, 
        input wire [N-1:0] B,
        input wire Ci,
        output wire [N-1:0] R,
        output wire Co
    );
    
    wire [N-2:0] c;
    genvar i;
    
    generate
        full_adder fa0 (.a(A[0]), .b(B[0]), .ci(Ci), .r(R[0]), .co(c[0]));
        
        for(i = 1; i < N-1; i = i + 1)
        begin
            full_adder fa (.a(A[i]), .b(B[i]), .ci(c[i-1]), .r(R[i]), .co(c[i]));
        end
        
        full_adder faN (.a(A[N-1]), .b(B[N-1]), .ci(c[N-2]), .r(R[N-1]), .co(Co));
    endgenerate

endmodule
