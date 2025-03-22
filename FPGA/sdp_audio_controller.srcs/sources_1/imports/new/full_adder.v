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


module full_adder(
        input a, b, ci,
        output r, co
    );
    wire t1, t2, t3;
    
    xor(t1, a, b);
    xor(r, ci, t1);
    
    and(t2, a, b);
    and(t3, t1, ci);
    or(co, t2, t3);
endmodule
