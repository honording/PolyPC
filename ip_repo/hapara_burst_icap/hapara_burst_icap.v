
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
	.I(din),
	.RDWRB(1'b0)
);


endmodule
