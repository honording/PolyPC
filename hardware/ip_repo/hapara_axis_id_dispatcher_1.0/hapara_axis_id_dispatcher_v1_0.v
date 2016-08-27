
`timescale 1 ns / 1 ps

	module hapara_axis_id_dispatcher_v1_0 #
	(
		// Users to add parameters here
        parameter integer NUM_SLAVES    = 1,
        parameter integer DATA_WIDTH    = 32

		// User parameters ends

	)
	(
		// Users to add ports here
        input wire [NUM_SLAVES-1: 0] priority_sel,

		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXIS
		input wire  s00_axis_aclk,
		input wire  s00_axis_aresetn,
		output wire  s00_axis_tready,
		input wire [DATA_WIDTH-1 : 0] s00_axis_tdata,
		input wire  s00_axis_tlast,
		input wire  s00_axis_tvalid,

		// Ports of Axi Master Bus Interface M00_AXIS
		input wire  m00_axis_aclk,
		input wire  m00_axis_aresetn,
		output wire  m00_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m00_axis_tdata,
		output wire  m00_axis_tlast,
		input wire  m00_axis_tready,

		// Ports of Axi Master Bus Interface M01_AXIS
		input wire  m01_axis_aclk,
		input wire  m01_axis_aresetn,
		output wire  m01_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m01_axis_tdata,
		output wire  m01_axis_tlast,
		input wire  m01_axis_tready,

		// Ports of Axi Master Bus Interface M02_AXIS
		input wire  m02_axis_aclk,
		input wire  m02_axis_aresetn,
		output wire  m02_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m02_axis_tdata,
		output wire  m02_axis_tlast,
		input wire  m02_axis_tready,

		// Ports of Axi Master Bus Interface M03_AXIS
		input wire  m03_axis_aclk,
		input wire  m03_axis_aresetn,
		output wire  m03_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m03_axis_tdata,
		output wire  m03_axis_tlast,
		input wire  m03_axis_tready,

		// Ports of Axi Master Bus Interface M04_AXIS
		input wire  m04_axis_aclk,
		input wire  m04_axis_aresetn,
		output wire  m04_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m04_axis_tdata,
		output wire  m04_axis_tlast,
		input wire  m04_axis_tready,

		// Ports of Axi Master Bus Interface M05_AXIS
		input wire  m05_axis_aclk,
		input wire  m05_axis_aresetn,
		output wire  m05_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m05_axis_tdata,
		output wire  m05_axis_tlast,
		input wire  m05_axis_tready,

		// Ports of Axi Master Bus Interface M06_AXIS
		input wire  m06_axis_aclk,
		input wire  m06_axis_aresetn,
		output wire  m06_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m06_axis_tdata,
		output wire  m06_axis_tlast,
		input wire  m06_axis_tready,

		// Ports of Axi Master Bus Interface M07_AXIS
		input wire  m07_axis_aclk,
		input wire  m07_axis_aresetn,
		output wire  m07_axis_tvalid,
		output wire [DATA_WIDTH-1 : 0] m07_axis_tdata,
		output wire  m07_axis_tlast,
		input wire  m07_axis_tready
	);

	localparam dispatch 	= 4'b0001;
	localparam waitslave	= 4'b0010;
	localparam terminate	= 4'b0100;
	localparam waitdata		= 4'b1000;

	reg [3 : 0] curr_state;
	reg [3 : 0] next_state;

	// Logic for curr_state
	always @(posedge s00_axis_aclk) begin
		if (!s00_axis_aresetn) begin
			// reset
			curr_state <= dispatch;
		end
		else begin
			curr_state <= next_state;
		end
	end

	wire slaves_ready;
	// Logic for next_state
	always @(s00_axis_tdata or slaves_ready or curr_state) begin 
		case (curr_state) 
			dispatch:
				if (s00_axis_tdata == {DATA_WIDTH{1'b1}}) begin
					next_state = waitslave;
				end
				else begin
					next_state = dispatch;
				end
			waitslave:
				if (slaves_ready) begin
					next_state = terminate;
				end
				else begin
					next_state = waitslave;
				end
			terminate:
				next_state = waitdata;
			waitdata: 
				if (s00_axis_tdata != {DATA_WIDTH{1'b1}}) begin
					next_state = dispatch;
				end
				else begin
					next_state = waitdata;
				end
			default:
				next_state = 3'bxxx;
		endcase
	end

	assign  m00_axis_tdata  = s00_axis_tdata;
	assign  m00_axis_tlast  = s00_axis_tlast;
	assign  m01_axis_tdata  = s00_axis_tdata;
	assign  m01_axis_tlast  = s00_axis_tlast;
	assign  m02_axis_tdata  = s00_axis_tdata;
	assign  m02_axis_tlast  = s00_axis_tlast;
	assign  m03_axis_tdata  = s00_axis_tdata;
	assign  m03_axis_tlast  = s00_axis_tlast;
	assign  m04_axis_tdata  = s00_axis_tdata;
	assign  m04_axis_tlast  = s00_axis_tlast;	
	assign  m05_axis_tdata  = s00_axis_tdata;
	assign  m05_axis_tlast  = s00_axis_tlast;
	assign  m06_axis_tdata  = s00_axis_tdata;
	assign  m06_axis_tlast  = s00_axis_tlast;
	assign  m07_axis_tdata  = s00_axis_tdata;
	assign  m07_axis_tlast  = s00_axis_tlast;

	// Add user logic here
    generate if (NUM_SLAVES == 1)
    begin: NUM_SLAVES_1
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  m00_axis_tready;

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid &&  
								   priority_sel[0]);

		assign  slaves_ready = m00_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 2)
    begin: NUM_SLAVES_2
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   priority_sel[1]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 3)
    begin: NUM_SLAVES_3
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   s00_axis_tvalid && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   priority_sel[2]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 4)
    begin: NUM_SLAVES_4
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready || 
        						   m03_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[3] && 
								   priority_sel[2]);

		assign  m03_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   priority_sel[3]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready &
							   m03_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 5)
    begin: NUM_SLAVES_5
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready || 
        						   m03_axis_tready || 
        						   m04_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   priority_sel[2]);

		assign  m03_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[4] && 
								   priority_sel[3]);

		assign  m04_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   priority_sel[4]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready &
							   m03_axis_tready &
							   m04_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 6)
    begin: NUM_SLAVES_6
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready || 
        						   m03_axis_tready || 
        						   m04_axis_tready || 
        						   m05_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   priority_sel[2]);

		assign  m03_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   priority_sel[3]);

		assign  m04_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[5] && 
								   priority_sel[4]);

		assign  m05_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   priority_sel[5]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready &
							   m03_axis_tready &
							   m04_axis_tready &
							   m05_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 7)
    begin: NUM_SLAVES_7
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready || 
        						   m03_axis_tready || 
        						   m04_axis_tready || 
        						   m05_axis_tready || 
        						   m06_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   priority_sel[2]);

		assign  m03_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   priority_sel[3]);

		assign  m04_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   priority_sel[4]);

		assign  m05_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[6] && 
								   priority_sel[5]);

		assign  m06_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   priority_sel[6]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready &
							   m03_axis_tready &
							   m04_axis_tready &
							   m05_axis_tready &
							   m06_axis_tready;
    end
    endgenerate

    generate if (NUM_SLAVES == 8)
    begin: NUM_SLAVES_8
        assign  s00_axis_tready = (curr_state == dispatch || curr_state == terminate) && 
        						  (m00_axis_tready || 
        						   m01_axis_tready || 
        						   m02_axis_tready || 
        						   m03_axis_tready || 
        						   m04_axis_tready || 
        						   m05_axis_tready || 
        						   m06_axis_tready || 
        						   m07_axis_tready);

		assign  m00_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   ~priority_sel[1] && 
								   priority_sel[0]);

		assign  m01_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   ~priority_sel[2] && 
								   priority_sel[1]);

		assign  m02_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   ~priority_sel[3] && 
								   priority_sel[2]);

		assign  m03_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   ~priority_sel[4] && 
								   priority_sel[3]);

		assign  m04_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch &&
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} && 
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   ~priority_sel[5] && 
								   priority_sel[4]);

		assign  m05_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   ~priority_sel[6] && 
								   priority_sel[5]);

		assign  m06_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   ~priority_sel[7] && 
								   priority_sel[6]);

		assign  m07_axis_tvalid = (curr_state == terminate) || 
								  (curr_state == dispatch && 
								   s00_axis_tdata != {DATA_WIDTH{1'b1}} &&
								   s00_axis_tvalid && 
								   priority_sel[7]);

		assign  slaves_ready = m00_axis_tready & 
							   m01_axis_tready &
							   m02_axis_tready &
							   m03_axis_tready &
							   m04_axis_tready &
							   m05_axis_tready &
							   m06_axis_tready &
							   m07_axis_tready;
    end
    endgenerate

	// User logic ends

	endmodule
