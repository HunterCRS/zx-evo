// This module receives 112 MHz as input clock
// and formes strobes for all clocked parts
// (now forms only 28 MHz strobes)

// sN		- Nth clock cycle out of 2
// qN		- Nth clock cycle out of 4
// cN		- Nth/4 clock cycle out of 16

//			0       1       2       3       0       1       2       3       0       1       2       3       
//			0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3 
// clk		-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
// s0		--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__--__
// q0		--______--______--______--______--______--______--______--______--______--______--______--______
// c0		--______________________________--______________________________--______________________________	(ex cbeg)
// c1		________--______________________________--______________________________--______________________	(ex post_cbeg)
// c2		________________--______________________________--______________________________--______________	(ex pre_cend)
// c3		________________________--______________________________--______________________________--______	(ex cend)


module clock (

	input wire clk,
	output reg clk175,
	output reg s0, q0, c0, c1, c2, c3

);


	reg [5:0] cnt = 0;

	always @(posedge clk)
		cnt <= cnt + 1;
		
		
	always @*
	begin

		s0 = cnt[0];
		q0 = cnt[1:0] == 2'd0;
		c0 = cnt[3:0] == 4'd0;
		c1 = cnt[3:0] == 4'd4;
		c2 = cnt[3:0] == 4'd8;
		c3 = cnt[3:0] == 4'd12;
		
		clk175 = cnt[5];
		
	end
	

endmodule
