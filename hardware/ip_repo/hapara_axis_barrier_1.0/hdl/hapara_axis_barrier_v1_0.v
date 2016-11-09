
`timescale 1 ns / 1 ps

    module hapara_axis_barrier_v1_0 #
    (
        // Users to add parameters here
        parameter     integer NUM_SLAVES    = 2,
        parameter     integer DATA_WIDTH    = 32
    )
    (
        // Ports of Axi Master Bus Interface M00_AXIS
        input wire  aclk,
        input wire  aresetn,

        input wire  m00_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m00_axis_tdata,
        output wire  m00_axis_tready,

        // Ports of Axi Master Bus Interface M01_AXIS
        input wire  m01_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m01_axis_tdata,
        output wire  m01_axis_tready,

        // Ports of Axi Master Bus Interface M02_AXIS
        input wire  m02_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m02_axis_tdata,
        output wire  m02_axis_tready,

        // Ports of Axi Master Bus Interface M03_AXIS
        input wire  m03_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m03_axis_tdata,
        output wire  m03_axis_tready,

        // Ports of Axi Master Bus Interface M04_AXIS
        input wire  m04_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m04_axis_tdata,
        output wire  m04_axis_tready,

        // Ports of Axi Master Bus Interface M05_AXIS
        input wire  m05_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m05_axis_tdata,
        output wire  m05_axis_tready,

        // Ports of Axi Master Bus Interface M06_AXIS
        input wire  m06_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m06_axis_tdata,
        output wire  m06_axis_tready,

        // Ports of Axi Master Bus Interface M07_AXIS
        input wire  m07_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m07_axis_tdata,
        output wire  m07_axis_tready,

        // Ports of Axi Master Bus Interface M08_AXIS
        input wire  m08_axis_tvalid,
        input wire [DATA_WIDTH-1 : 0] m08_axis_tdata,
        output wire  m08_axis_tready,

        // Connect to as master to slaves
        output wire  s00_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s00_axis_tdata,
        input wire  s00_axis_tready,

        // Ports of Axi Master Bus Interface s01_AXIS
        output wire  s01_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s01_axis_tdata,
        input wire  s01_axis_tready,

        // Ports of Axi Master Bus Interface s02_AXIS
        output wire  s02_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s02_axis_tdata,
        input wire  s02_axis_tready,

        // Ports of Axi Master Bus Interface s03_AXIS
        output wire  s03_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s03_axis_tdata,
        input wire  s03_axis_tready,

        // Ports of Axi Master Bus Interface s04_AXIS
        output wire  s04_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s04_axis_tdata,
        input wire  s04_axis_tready,

        // Ports of Axi Master Bus Interface s05_AXIS
        output wire  s05_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s05_axis_tdata,
        input wire  s05_axis_tready,

        // Ports of Axi Master Bus Interface s06_AXIS
        output wire  s06_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s06_axis_tdata,
        input wire  s06_axis_tready,

        // Ports of Axi Master Bus Interface s07_AXIS
        output wire  s07_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s07_axis_tdata,
        input wire  s07_axis_tready,

        // Ports of Axi Master Bus Interface s08_AXIS
        output wire  s08_axis_tvalid,
        output wire [DATA_WIDTH-1 : 0] s08_axis_tdata,
        input wire  s08_axis_tready

    );
    

    // Add user logic here
    // assumption NUM_SLAVES is alway greater than 0
    // clk and reset can use the first master port
    wire notRelease;
    wire syn;

    generate if (NUM_SLAVES == 1) 
    begin: NUM_SLAVES_1
        assign notRelease = m00_axis_tvalid;
        assign syn = m00_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 2)
    begin: NUM_SLAVES_2
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate
   
    generate if (NUM_SLAVES == 3)
    begin: NUM_SLAVES_3
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid | m02_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd1;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 4)
    begin: NUM_SLAVES_4
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid | 
                            m02_axis_tvalid | m03_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;                

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 5)
    begin: NUM_SLAVES_5
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid |
                            m02_axis_tvalid | m03_axis_tvalid |
                            m04_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid & m04_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m04_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;   
        assign s04_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;                

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;
        assign s04_axis_tdata  = 32'd4;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 6)
    begin: NUM_SLAVES_6
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid |
                            m02_axis_tvalid | m03_axis_tvalid |
                            m04_axis_tvalid | m05_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid & m04_axis_tvalid & m05_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m04_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m05_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;   
        assign s04_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s05_axis_tvalid = (curr_state == releasing)?1'b1:1'b0; 

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;
        assign s04_axis_tdata  = 32'd4;
        assign s05_axis_tdata  = 32'd5;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 7) 
    begin: NUM_SLAVES_7
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid |
                            m02_axis_tvalid | m03_axis_tvalid |
                            m04_axis_tvalid | m05_axis_tvalid |
                            m06_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid & m04_axis_tvalid & m05_axis_tvalid &
                     m06_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m04_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m05_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m06_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;   
        assign s04_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s05_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s06_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;
        assign s04_axis_tdata  = 32'd4;
        assign s05_axis_tdata  = 32'd5;
        assign s06_axis_tdata  = 32'd6;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 8)
    begin: NUM_SLAVES_8
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid |
                            m02_axis_tvalid | m03_axis_tvalid |
                            m04_axis_tvalid | m05_axis_tvalid |
                            m06_axis_tvalid | m07_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid & m04_axis_tvalid & m05_axis_tvalid &
                     m06_axis_tvalid & m07_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m04_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m05_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m06_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m07_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s04_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s05_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s06_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s07_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;
        assign s04_axis_tdata  = 32'd4;
        assign s05_axis_tdata  = 32'd5;
        assign s06_axis_tdata  = 32'd6;
        assign s07_axis_tdata  = 32'd7;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate

    generate if (NUM_SLAVES == 9)
    begin: NUM_SLAVES_9
        assign notRelease = m00_axis_tvalid | m01_axis_tvalid |
                            m02_axis_tvalid | m03_axis_tvalid |
                            m04_axis_tvalid | m05_axis_tvalid |
                            m06_axis_tvalid | m07_axis_tvalid |
                            m08_axis_tvalid;
        assign syn = m00_axis_tvalid & m01_axis_tvalid & m02_axis_tvalid &
                     m03_axis_tvalid & m04_axis_tvalid & m05_axis_tvalid &
                     m06_axis_tvalid & m07_axis_tvalid & m08_axis_tvalid;
        assign m00_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m01_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m02_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m03_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m04_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m05_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m06_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m07_axis_tready = (curr_state == locking)?1'b1:1'b0;
        assign m08_axis_tready = (curr_state == locking)?1'b1:1'b0;

        assign s00_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s01_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s02_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s03_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;   
        assign s04_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s05_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s06_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s07_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;
        assign s08_axis_tvalid = (curr_state == releasing)?1'b1:1'b0;

        assign s00_axis_tdata  = 32'd0;
        assign s01_axis_tdata  = 32'd1;
        assign s02_axis_tdata  = 32'd2;
        assign s03_axis_tdata  = 32'd3;
        assign s04_axis_tdata  = 32'd4;
        assign s05_axis_tdata  = 32'd5;
        assign s06_axis_tdata  = 32'd6;
        assign s07_axis_tdata  = 32'd7;
        assign s08_axis_tdata  = 32'd7;

        // wire notRelease;
        // wire syn;

        localparam waiting      = 2'b01;
        localparam locking      = 2'b10;
        localparam releasing    = 2'b11;

        reg [1 : 0] curr_state;
        reg [1 : 0] next_state;

        always @(posedge aclk or negedge aresetn) begin
            if (!aresetn) begin
                // reset
                curr_state <= waiting;
            end
            else begin
                curr_state <= next_state;
            end
        end

        always @(curr_state or syn or notRelease) begin
            case(curr_state)
                waiting:
                    if (syn) begin
                        next_state = locking;
                    end 
                    else begin
                        next_state = waiting;
                    end
                locking:
                    if (!notRelease) begin
                        next_state = releasing;
                    end
                    else begin
                        next_state = locking;
                    end
                releasing:
                    next_state = waiting;
                default:
                    next_state = 2'bxx;    
            endcase
        end
    end
    endgenerate



    endmodule
