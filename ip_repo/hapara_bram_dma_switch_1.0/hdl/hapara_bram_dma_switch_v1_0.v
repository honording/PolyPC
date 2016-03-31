
`timescale 1 ns / 1 ps

	module hapara_bram_dma_switch_v1_0 #
	(
		// Users to add parameters here
        parameter integer BRAM_ADDR_WIDTH = 14,
		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 4
		
	)
	(
		// Users to add ports here
		input   [BRAM_ADDR_WIDTH - 1 : 0] addr_ctrl,
        input   [C_S00_AXI_DATA_WIDTH - 1 : 0] data_in_ctrl,
        output  [C_S00_AXI_DATA_WIDTH - 1 : 0] data_out_ctrl,
        input   [C_S00_AXI_DATA_WIDTH / 8 - 1 : 0] we_ctrl,
        input   clk_ctrl,
        input   rst_ctrl,
        input   en_ctrl,
		
        output  [BRAM_ADDR_WIDTH - 1 : 0] addr_inst,
        output  [C_S00_AXI_DATA_WIDTH - 1 : 0] data_in_inst,
        input   [C_S00_AXI_DATA_WIDTH - 1 : 0] data_out_inst,
        output  [C_S00_AXI_DATA_WIDTH / 8 - 1 : 0] we_inst,
        output  clk_inst,
        output  rst_inst,
        output  en_inst,
        
        output  [BRAM_ADDR_WIDTH - 1 : 0] addr_data,
        output  [C_S00_AXI_DATA_WIDTH - 1 : 0] data_in_data,
        input   [C_S00_AXI_DATA_WIDTH - 1 : 0] data_out_data,
        output  [C_S00_AXI_DATA_WIDTH / 8 - 1 : 0] we_data,
        output  clk_data,
        output  rst_data,
        output  en_data,
       
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready
	);
// Instantiation of Axi Bus Interface S00_AXI
	wire [C_S00_AXI_DATA_WIDTH - 1 : 0] base_full;
    wire [C_S00_AXI_DATA_WIDTH - 1 : 0] length_full;
    
	hapara_bram_dma_switch_v1_0_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) hapara_bram_dma_switch_v1_0_S00_AXI_inst (
	    .base(base_full),
	    .length(length_full),
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready)
	);

	// Add user logic here

	
	wire [BRAM_ADDR_WIDTH - 1 : 0] base_addr;
	wire [BRAM_ADDR_WIDTH - 1 : 0] length_addr;
	
	assign base_addr   = base_full[BRAM_ADDR_WIDTH - 1 : 0];
	assign length_addr = length_full[BRAM_ADDR_WIDTH - 1 : 0];
	
	localparam BYTE_WIDTH = C_S00_AXI_DATA_WIDTH / 8;
	
    assign  clk_inst = clk_ctrl;
    assign  clk_data = clk_ctrl;
    assign  rst_inst = rst_ctrl;
    assign  rst_data = rst_ctrl;
    assign  we_inst = ((addr_ctrl >= base_addr) && (addr_ctrl < (base_addr + length_addr)))? 
                      we_ctrl : 
                      {BYTE_WIDTH{1'b0}};
    assign  we_data = we_ctrl;
    assign  addr_inst = addr_ctrl;
    assign  addr_data = addr_ctrl;
    assign  data_in_inst = data_in_ctrl;
    assign  data_in_data = data_in_ctrl;
    assign  en_inst = en_ctrl;
    assign  en_data = en_ctrl;
    
    assign  data_out_ctrl = data_out_data;
    
    
	// User logic ends

	endmodule
