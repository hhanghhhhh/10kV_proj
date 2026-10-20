/************************************************
//FILE NAME:        com_between_fpga_slow_rx.v
//DESCRIPTION:      com_between_fpga_slow_rx
//AUTHOR:           hehang
//DATE:             25/06/20
//LOG:              
************************************************/
//************** Module Define *******************//
module com_between_fpga_slow_rx
#(
    parameter   DATA_NUM        =       2           ,
    parameter   OVERITME_CNT    =       16'd80        //10 * 8
)
(   
    input                       sys_clk             ,  // 200M
    input                       sys_rst_n           ,
    input                       rx_clk              ,  // 对端发来的 clk
    input                       rx_data             ,  // 对端发来的 data
    output reg                  valid               ,  // 数据接收成功标志
    output reg [31:0]           err_cnt             ,  // 错误计数
    output reg [31:0]           rx_data1            ,
    output reg [31:0]           rx_data2            ,
    output reg [31:0]           rx_data3            ,
    output reg [31:0]           rx_data4            ,
    output reg [31:0]           rx_data5            ,
    output reg [31:0]           rx_data6            ,
    output reg [31:0]           rx_data7            ,
    output reg [31:0]           rx_data8            ,
    output reg [31:0]           rx_data9            ,
    output reg [31:0]           rx_data10            
);

//*************** Parameter & Internal Register ****//
// parameter
localparam  FRAME_HEAD        = 32'h5555AAAA;
localparam  FRAME_TAIL        = 32'hAAAA5555;
localparam  TOTAL_U32S        = DATA_NUM + 3;

// 接收状态机
localparam  S_IDEL            = 4'b0000;
localparam  S_RECV_WORD       = 4'b0001;
localparam  S_CHECK           = 4'b0010;




// reg
reg   [3:0]     state                   ;
reg   [7:0]     bit_cnt                 ;
reg   [7:0]     word_cnt                ;
reg   [31:0]    shift_reg               ;
reg   [31:0]    buffer[0:TOTAL_U32S-1]  ;
reg   [31:0]    checksum                ;
reg   [15:0]    overtime_cnt            ;
wire            overtime_flag           ;
reg             rx_data_d1              ;
reg             rx_clk_d1               ;
reg             rx_clk_d2               ;
wire            rx_clk_pedage           ;
//*************** Main Code ***********************//
// 用于采样 rx_clk 边沿
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        rx_clk_d1 <= 1'b0;
        rx_clk_d2 <= 1'b0;
    end else begin
        rx_clk_d1 <= rx_clk;
        rx_clk_d2 <= rx_clk_d1;
    end
end
assign rx_clk_pedage = (rx_clk_d1 == 1'b1) && (rx_clk_d2 == 1'b0);

// 接收数据同步
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        rx_data_d1 <= 1'b0;
    end else begin
        rx_data_d1 <= rx_data;
    end
end

// 超时计数
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        overtime_cnt <= 16'd0;
    end else begin
        if (rx_clk_pedage) begin
            overtime_cnt <= 16'd0;
        end else begin
            if (overtime_cnt < OVERITME_CNT) begin
                overtime_cnt <= overtime_cnt + 1'b1;
            end
        end
    end
end

assign  overtime_flag = (overtime_cnt < OVERITME_CNT) ? 1'b0 : 1'b1;


always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        state <= S_IDEL;
        valid <= 1'b0;
        shift_reg <= 32'd0;
        bit_cnt <= 8'd0;
        word_cnt <= 8'd0;
        checksum <= 32'd0;
        rx_data1  <= 32'd0;
        rx_data2  <= 32'd0;
        rx_data3  <= 32'd0;
        rx_data4  <= 32'd0;
        rx_data5  <= 32'd0;
        rx_data6  <= 32'd0;
        rx_data7  <= 32'd0;
        rx_data8  <= 32'd0;
        rx_data9  <= 32'd0;
        rx_data10 <= 32'd0;
        err_cnt   <= 32'd0;
    end else begin
        case (state)
            S_IDEL: begin
                valid <= 1'b0;
                if (rx_clk_pedage) begin
                    state <= S_RECV_WORD;
                    shift_reg <= {31'd0, rx_data_d1};
                    bit_cnt <= 8'd1;
                    word_cnt <= 8'd0;
                    checksum <= 32'd0;
                end
            end
            S_RECV_WORD: begin
                if (overtime_flag) begin
                    bit_cnt <= 8'd0;
                    word_cnt <= 8'd0;
                    checksum <= 32'd0;
                    shift_reg <= 32'd0;
                    state <= S_IDEL;
                end else begin
                    if (rx_clk_pedage) begin
                        shift_reg <= {shift_reg[30:0], rx_data_d1};
                        
                        if (bit_cnt == 31) begin
                            bit_cnt <= 0;

                            buffer[word_cnt] <= {shift_reg[30:0], rx_data_d1};

                            // 累加前 DATA_NUM + 1 个字段
                            if (word_cnt < DATA_NUM + 1)
                                checksum <= checksum + {shift_reg[30:0], rx_data_d1};

                            
                            if (word_cnt == TOTAL_U32S - 1) begin
                            // 收完 checksum
                                word_cnt <= 0;
                                state <= S_CHECK;
                            end else begin
                                word_cnt <= word_cnt + 1;
                            end
                        end else begin
                            bit_cnt <= bit_cnt + 1;
                        end
                    end
                end
            end
            S_CHECK: begin
                if (buffer[0] == FRAME_HEAD && buffer[DATA_NUM + 1] == checksum && buffer[DATA_NUM + 2] == FRAME_TAIL) begin
                    // 与发送端对应，暂时预留10个数据
                    if (DATA_NUM > 0)   rx_data1   <= buffer[1] ;
                    if (DATA_NUM > 1)   rx_data2   <= buffer[2] ;
                    if (DATA_NUM > 2)   rx_data3   <= buffer[3] ;
                    if (DATA_NUM > 3)   rx_data4   <= buffer[4] ;
                    if (DATA_NUM > 4)   rx_data5   <= buffer[5] ;
                    if (DATA_NUM > 5)   rx_data6   <= buffer[6] ;
                    if (DATA_NUM > 6)   rx_data7   <= buffer[7] ;
                    if (DATA_NUM > 7)   rx_data8   <= buffer[8] ;
                    if (DATA_NUM > 8)   rx_data9   <= buffer[9] ;
                    if (DATA_NUM > 9)   rx_data10  <= buffer[10];
                    valid <= 1;
                end
                else begin
                    err_cnt <= err_cnt + 1'b1;
                end
                state <= S_IDEL;
            end
            default: state <= S_IDEL;
        endcase
    end
end










endmodule
//*************** END OF LINE ********************//