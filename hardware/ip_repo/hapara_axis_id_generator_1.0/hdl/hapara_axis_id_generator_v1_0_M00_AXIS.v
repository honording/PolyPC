
`timescale 1 ns / 1 ps

    module hapara_axis_id_generator_v1_0_M00_AXIS #
    (
        // Users to add parameters here

        // User parameters ends
        // Do not modify the parameters beyond this line

        // Width of S_AXIS address bus. The slave accepts the read and write addresses of width C_M_AXIS_TDATA_WIDTH.
        parameter integer C_M_AXIS_TDATA_WIDTH    = 32
        // Start count is the numeber of clock cycles the master will wait before initiating/issuing any transaction.
        // parameter integer C_M_START_COUNT    = 32
    )
    (
        // Users to add ports here
        input wire En,
        output wire Finish,

        // input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] orgX,
        // input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] orgY,
        // input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] lengthX,
        // input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] lengthY,

        input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] org,
        input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] len,
        input wire [C_M_AXIS_TDATA_WIDTH - 1 : 0] numOfSlv,

        // User ports ends
        // Do not modify the ports beyond this line

        // Global ports
        input wire  M_AXIS_ACLK,
        // 
        input wire  M_AXIS_ARESETN,
        // Master Stream Ports. TVALID indicates that the master is driving a valid transfer, A transfer takes place when both TVALID and TREADY are asserted. 
        output wire  M_AXIS_TVALID,
        // TDATA is the primary payload that is used to provide the data that is passing across the interface from the master.
        output wire [C_M_AXIS_TDATA_WIDTH-1 : 0] M_AXIS_TDATA,
        // TSTRB is the byte qualifier that indicates whether the content of the associated byte of TDATA is processed as a data byte or a position byte.
        //output wire [(C_M_AXIS_TDATA_WIDTH/8)-1 : 0] M_AXIS_TSTRB,
        // TLAST indicates the boundary of a packet.
        output wire  M_AXIS_TLAST,
        // TREADY indicates that the slave can accept a transfer in the current cycle.
        input wire  M_AXIS_TREADY
    );

    // Maxinum lenght for vertical X 
    localparam X_LENGTH = C_M_AXIS_TDATA_WIDTH / 2;
    // Maxinum lenght for horizontal Y 
    localparam Y_LENGTH = C_M_AXIS_TDATA_WIDTH / 2;

    localparam LENGTH   = C_M_AXIS_TDATA_WIDTH / 2;

    // localparam numOfSlave   = 2;

    // State defination
    localparam reset            = 3'b001; 
    localparam counting         = 3'b010;
    localparam ending           = 3'b100;

    reg [2 : 0] next_state;
    reg [2 : 0] curr_state;

    reg [X_LENGTH - 1 : 0] counterX;
    reg [Y_LENGTH - 1 : 0] counterY;

    wire [LENGTH - 1 : 0] orgX;
    wire [LENGTH - 1 : 0] orgY;
    wire [LENGTH - 1 : 0] lengthX;
    wire [LENGTH - 1 : 0] lengthY;
    assign orgX     = org[C_M_AXIS_TDATA_WIDTH - 1 : LENGTH];
    assign orgY     = org[LENGTH - 1 : 0];
    assign lengthX  = len[C_M_AXIS_TDATA_WIDTH - 1 : LENGTH];
    assign lengthY  = len[LENGTH - 1 : 0];

    assign Finish = curr_state == ending;

    // next state
    always @(curr_state or En or slave_counter) begin
        case (curr_state)
            reset:
                if (En) begin
                    next_state = counting;
                end
                else begin
                    next_state = reset;
                end
            counting:
                if (slave_counter == 1) begin
                    next_state = ending;
                end
                else begin
                    next_state = counting;
                end
            ending:
                if (M_AXIS_TREADY == 1) begin
                    next_state = reset;
                end
                else begin
                    next_state = ending;
                end
            default:
                next_state = 3'bxxx;
        endcase
    end
    assign M_AXIS_TVALID = (curr_state == counting || curr_state == ending);

    // logic for current state
    always @(posedge M_AXIS_ACLK) begin
        if (!M_AXIS_ARESETN) begin
            // reset
            curr_state <= reset;
        end
        else begin
            curr_state <= next_state;
        end
    end

    // logic for counterX, counterY
    always @(posedge M_AXIS_ACLK) begin
        if (!M_AXIS_ARESETN || curr_state == reset) begin
            counterX <= {X_LENGTH{1'b0}};
            counterY <= {Y_LENGTH{1'b0}};
        end
        else if (curr_state == counting || curr_state == ending) begin
            if (M_AXIS_TREADY) begin
                if (counterY >= lengthY - 1 && counterX >= lengthX - 1) begin
                    counterX <= {X_LENGTH{1'b1}};
                    counterY <= {Y_LENGTH{1'b1}};
                end
                else if (counterY >= lengthY - 1) begin
                    counterX <= counterX + 1;
                    counterY <= {Y_LENGTH{1'b0}};
                end
                else begin
                    counterY <= counterY + 1;
                end
            end
        end 
        else begin
            counterX <= counterX;
            counterY <= counterY;
        end 
    end

    //logic to count how many slaves reveived the termination
    reg [C_M_AXIS_TDATA_WIDTH - 1 : 0] slave_counter;

    always @(posedge M_AXIS_ACLK) begin
        if (!M_AXIS_ARESETN || curr_state == reset) begin
            // reset
            slave_counter <= 0;
        end
        else if ((curr_state == counting || curr_state == ending) && 
                 M_AXIS_TREADY && 
                 counterY == {Y_LENGTH{1'b1}} && counterX == {X_LENGTH{1'b1}}) begin
            slave_counter <= slave_counter + 1;
        end
        else begin
            slave_counter <= slave_counter;
        end
    end

    wire [X_LENGTH - 1 : 0] resultX;
    wire [Y_LENGTH - 1 : 0] resultY;
    assign resultX = (counterX == {X_LENGTH{1'b1}})?{X_LENGTH{1'b1}}:counterX + orgX;
    assign resultY = (counterY == {Y_LENGTH{1'b1}})?{Y_LENGTH{1'b1}}:counterY + orgY;
    assign M_AXIS_TDATA = {resultX, resultY};
    
    assign M_AXIS_TLAST = 1'b0;
    endmodule
