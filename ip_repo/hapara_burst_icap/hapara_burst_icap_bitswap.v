
`timescale 1 ns / 1 ps
module hapara_burst_icap_bitswap (
    parameter integer DATA_WIDTH = 8
)
(
    input   wire [DATA_WIDTH - 1 : 0] din,
    output  wire [DATA_WIDTH - 1 : 0] dout
);

assign dout = {din[0], din[1], din[2], din[3],
               din[4], din[5], din[6], din[7]};

endmodule