
`timescale 1 ns / 1 ps

module hapara_burst_icap #
(
	parameter integer DATA_WIDTH = 32
)
(
	input  clk,
	input  rst,
	input  en,
	input  [DATA_WIDTH / 8 - 1 : 0] we,
	input  [DATA_WIDTH - 1 : 0] addr,
	input  [DATA_WIDTH - 1 : 0] din,
	output  [DATA_WIDTH - 1 : 0] dout
);

wire ready = ~((&we) & en);

wire [DATA_WIDTH - 1 : 0] swap_little;

localparam NUM_OF_BYTE = DATA_WIDTH / 8;
generate
	genvar i;
	for (i = 0; i < NUM_OF_BYTE; i = i + 1) begin
		hapara_burst_icap_bitswap bitswap_bit (
			.din(din[8 * (i + 1) - 1: 8 * i]),
			.dout(swap_little[8 * (4 - i) - 1: 8 * (3 - i)])
		);
	end
endgenerate

ICAPE2 #
(
	.DEVICE_ID(0'h3651093),
	.ICAP_WIDTH("X32"),
	.SIM_CFG_FILE_NAME("None")
)
ICAPE2_inst (
	.O(),
	.CLK(clk),
	.CSIB(ready),
	.I(swap_little),
	.RDWRB(1'b0)
);


endmodule
