`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/04/2016 09:15:54 PM
// Design Name: 
// Module Name: hapara_lmb_dma_dup
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


module hapara_lmb_dma_dup #(
        parameter integer DATA_WIDTH = 32,
        parameter integer NUM_SLAVE = 4
    )
    (
//----------------FROM DMA BRAM CONTROLLER-----------------
		input   [DATA_WIDTH - 1 : 0] addr_ctrl,
        input   [DATA_WIDTH - 1 : 0] data_in_ctrl,
        output  [DATA_WIDTH - 1 : 0] data_out_ctrl,
        input   [DATA_WIDTH / 8 - 1 : 0] we_ctrl,
        input   clk_ctrl,
        input   rst_ctrl,
        input   en_ctrl,

//----------------FROM SLAVE 0-----------------
		input   [DATA_WIDTH - 1 : 0] addr_s0,
        input   [DATA_WIDTH - 1 : 0] data_in_s0,
        output  [DATA_WIDTH - 1 : 0] data_out_s0,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s0,
        input   clk_s0,
        input   rst_s0,
        input   en_s0,
        
//----------------FROM SLAVE 1-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s1,
        input   [DATA_WIDTH - 1 : 0] data_in_s1,
        output  [DATA_WIDTH - 1 : 0] data_out_s1,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s1,
        input   clk_s1,
        input   rst_s1,
        input   en_s1,
        
//----------------FROM SLAVE 2-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s2,
        input   [DATA_WIDTH - 1 : 0] data_in_s2,
        output  [DATA_WIDTH - 1 : 0] data_out_s2,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s2,
        input   clk_s2,
        input   rst_s2,
        input   en_s2,
                
//----------------FROM SLAVE 3-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s3,
        input   [DATA_WIDTH - 1 : 0] data_in_s3,
        output  [DATA_WIDTH - 1 : 0] data_out_s3,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s3,
        input   clk_s3,
        input   rst_s3,
        input   en_s3,

//----------------FROM SLAVE 4-----------------
		input   [DATA_WIDTH - 1 : 0] addr_s4,
        input   [DATA_WIDTH - 1 : 0] data_in_s4,
        output  [DATA_WIDTH - 1 : 0] data_out_s4,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s4,
        input   clk_s4,
        input   rst_s4,
        input   en_s4,
        
//----------------FROM SLAVE 5-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s5,
        input   [DATA_WIDTH - 1 : 0] data_in_s5,
        output  [DATA_WIDTH - 1 : 0] data_out_s5,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s5,
        input   clk_s5,
        input   rst_s5,
        input   en_s5,
        
//----------------FROM SLAVE 6-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s6,
        input   [DATA_WIDTH - 1 : 0] data_in_s6,
        output  [DATA_WIDTH - 1 : 0] data_out_s6,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s6,
        input   clk_s6,
        input   rst_s6,
        input   en_s6,
                
//----------------FROM SLAVE 7-----------------
        input   [DATA_WIDTH - 1 : 0] addr_s7,
        input   [DATA_WIDTH - 1 : 0] data_in_s7,
        output  [DATA_WIDTH - 1 : 0] data_out_s7,
        input   [DATA_WIDTH / 8 - 1 : 0] we_s7,
        input   clk_s7,
        input   rst_s7,
        input   en_s7,
                
//----------------TO BRAM 0-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b0,
        output  [DATA_WIDTH - 1 : 0] data_in_b0,
        input   [DATA_WIDTH - 1 : 0] data_out_b0,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b0,
        output  clk_b0,
        output  rst_b0,
        output  en_b0,
        
//----------------TO BRAM 1-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b1,
        output  [DATA_WIDTH - 1 : 0] data_in_b1,
        input   [DATA_WIDTH - 1 : 0] data_out_b1,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b1,
        output  clk_b1,
        output  rst_b1,
        output  en_b1,       

//----------------TO BRAM 2-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b2,
        output  [DATA_WIDTH - 1 : 0] data_in_b2,
        input   [DATA_WIDTH - 1 : 0] data_out_b2,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b2,
        output  clk_b2,
        output  rst_b2,
        output  en_b2,
        
//----------------TO BRAM 3-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b3,
        output  [DATA_WIDTH - 1 : 0] data_in_b3,
        input   [DATA_WIDTH - 1 : 0] data_out_b3,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b3,
        output  clk_b3,
        output  rst_b3,
        output  en_b3,

//----------------TO BRAM 4-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b4,
        output  [DATA_WIDTH - 1 : 0] data_in_b4,
        input   [DATA_WIDTH - 1 : 0] data_out_b4,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b4,
        output  clk_b4,
        output  rst_b4,
        output  en_b4,
        
//----------------TO BRAM 5-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b5,
        output  [DATA_WIDTH - 1 : 0] data_in_b5,
        input   [DATA_WIDTH - 1 : 0] data_out_b5,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b5,
        output  clk_b5,
        output  rst_b5,
        output  en_b5,       

//----------------TO BRAM 6-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b6,
        output  [DATA_WIDTH - 1 : 0] data_in_b6,
        input   [DATA_WIDTH - 1 : 0] data_out_b6,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b6,
        output  clk_b6,
        output  rst_b6,
        output  en_b6,
        
//----------------TO BRAM 7-----------------        
        output  [DATA_WIDTH - 1 : 0] addr_b7,
        output  [DATA_WIDTH - 1 : 0] data_in_b7,
        input   [DATA_WIDTH - 1 : 0] data_out_b7,
        output  [DATA_WIDTH / 8 - 1 : 0] we_b7,
        output  clk_b7,
        output  rst_b7,
        output  en_b7

    );
    wire dma_tran;
    localparam NUM_BYTE = DATA_WIDTH / 8;
    assign dma_tran = en_ctrl && (we_ctrl == {NUM_BYTE{1'b1}});
    assign data_out_ctrl = {DATA_WIDTH{1'b0}};

    generate if (NUM_SLAVE > 0) 
    begin
        assign clk_b0           = clk_s0;                                       //output
        assign rst_b0           = rst_s0;                                       //output
        assign data_out_s0      = data_out_b0;                                  //input
        assign addr_b0          = (dma_tran == 1'b1)?addr_ctrl:addr_s0;         //output
        assign data_in_b0       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s0;   //output
        assign we_b0            = (dma_tran == 1'b1)?we_ctrl:we_s0;             //output
        assign en_b0            = (dma_tran == 1'b1)?en_ctrl:en_s0;             //output
    end
    endgenerate
    
    generate if (NUM_SLAVE > 1) 
    begin
        assign clk_b1           = clk_s1;
        assign rst_b1           = rst_s1;
        assign data_out_s1      = data_out_b1;
        assign addr_b1          = (dma_tran == 1'b1)?addr_ctrl:addr_s1;
        assign data_in_b1       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s1;
        assign we_b1            = (dma_tran == 1'b1)?we_ctrl:we_s1;
        assign en_b1            = (dma_tran == 1'b1)?en_ctrl:en_s1;
    end
    endgenerate

    generate if (NUM_SLAVE > 2) 
    begin
        assign clk_b2           = clk_s2;
        assign rst_b2           = rst_s2;
        assign data_out_s2      = data_out_b2;
        assign addr_b2          = (dma_tran == 1'b1)?addr_ctrl:addr_s2;
        assign data_in_b2       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s2;
        assign we_b2            = (dma_tran == 1'b1)?we_ctrl:we_s2;
        assign en_b2            = (dma_tran == 1'b1)?en_ctrl:en_s2;
    end
    endgenerate

    generate if (NUM_SLAVE > 3) 
    begin
        assign clk_b3           = clk_s3;
        assign rst_b3           = rst_s3;
        assign data_out_s3      = data_out_b3;
        assign addr_b3          = (dma_tran == 1'b1)?addr_ctrl:addr_s3;
        assign data_in_b3       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s3;
        assign we_b3            = (dma_tran == 1'b1)?we_ctrl:we_s3;
        assign en_b3            = (dma_tran == 1'b1)?en_ctrl:en_s3;
    end
    endgenerate

    generate if (NUM_SLAVE > 4) 
    begin
        assign clk_b4           = clk_s4;
        assign rst_b4           = rst_s4;
        assign data_out_s4      = data_out_b4;
        assign addr_b4          = (dma_tran == 1'b1)?addr_ctrl:addr_s4;
        assign data_in_b4       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s4;
        assign we_b4            = (dma_tran == 1'b1)?we_ctrl:we_s4;
        assign en_b4            = (dma_tran == 1'b1)?en_ctrl:en_s4;
    end
    endgenerate

    generate if (NUM_SLAVE > 5) 
    begin
        assign clk_b5           = clk_s5;
        assign rst_b5           = rst_s5;
        assign data_out_s5      = data_out_b5;
        assign addr_b5          = (dma_tran == 1'b1)?addr_ctrl:addr_s5;
        assign data_in_b5       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s5;
        assign we_b5            = (dma_tran == 1'b1)?we_ctrl:we_s5;
        assign en_b5            = (dma_tran == 1'b1)?en_ctrl:en_s5;
    end
    endgenerate

    generate if (NUM_SLAVE > 6) 
    begin
        assign clk_b6           = clk_s6;
        assign rst_b6           = rst_s6;
        assign data_out_s6      = data_out_b6;
        assign addr_b6          = (dma_tran == 1'b1)?addr_ctrl:addr_s6;
        assign data_in_b6       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s6;
        assign we_b6            = (dma_tran == 1'b1)?we_ctrl:we_s6;
        assign en_b6            = (dma_tran == 1'b1)?en_ctrl:en_s6;
    end
    endgenerate

    generate if (NUM_SLAVE > 7) 
    begin
        assign clk_b7           = clk_s7;
        assign rst_b7           = rst_s7;
        assign data_out_s7      = data_out_b7;
        assign addr_b7          = (dma_tran == 1'b1)?addr_ctrl:addr_s7;
        assign data_in_b7       = (dma_tran == 1'b1)?data_in_ctrl:data_in_s7;
        assign we_b7            = (dma_tran == 1'b1)?we_ctrl:we_s7;
        assign en_b7            = (dma_tran == 1'b1)?en_ctrl:en_s7;
    end
    endgenerate
endmodule
