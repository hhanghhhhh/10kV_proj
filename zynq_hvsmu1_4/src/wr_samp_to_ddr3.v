/************************************************
//FILE NAME:        wr_samp_to_ddr3.v
//DESCRIPTION:      wr_samp_to_ddr3
//AUTHOR:           hehang
//DATE:             25/09/29
//LOG:              将电流、电压采样值依次写入 fifo 并添加不同的标签用于识别
************************************************/
//************** Module Define *******************//
module wr_samp_to_ddr3
(   
    input                       sys_clk             ,  // xxM
    input                       sys_rst_n           ,
    input                       en                  ,
    input        [23:0]         vol_samp            ,
    input        [23:0]         cur_samp            ,
    input                       update_flag         ,
    input        [ 1:0]         range_sw            ,
    output  reg                 valid               ,
    output  reg                 wrfifo_clr          ,
    output  wire                wrfifo_clk          ,
    output  reg  [15:0]         wrfifo_din          ,
    output  reg                 wrfifo_wren         
);

//*************** Parameter & Internal Register ****//
// parameter
// 用高8位作为标识
localparam [7:0] VOL_TAG = 8'hA5;  // 电压标识
localparam [7:0] CUR_TAG0 = 8'h5A;  // 电流标识
localparam [7:0] CUR_TAG1 = 8'h5B;  // 电流标识
localparam [7:0] CUR_TAG2 = 8'h5C;  // 电流标识
localparam [7:0] CUR_TAG3 = 8'h5D;  // 电流标识
localparam ST_IDLE   = 3'd0;  // 空闲，等待en=1
localparam ST_RST    = 3'd1;  // fifo复位
localparam ST_WAIT   = 3'd2;  // 等待复位完成
localparam ST_PREP   = 3'd3;  // 准备写数据
localparam ST_VHIGH  = 3'd4;  // 写电压高16位
localparam ST_VLOW   = 3'd5;  // 写电压低16位
localparam ST_CHIGH  = 3'd6;  // 写电流高16位
localparam ST_CLOW   = 3'd7;  // 写电流低16位



// reg
reg [2:0]  state;         
reg [31:0] vol_buf;       // 缓存电压
reg [31:0] cur_buf;       // 缓存电流
reg [7:0]  rst_cnt;       // 复位等待计数
wire[7:0]  cur_tag;
wire[7:0]  vol_tag;
//***************** Assign ***********************//
assign wrfifo_clk = sys_clk;


//*************** Main Code ***********************//
assign vol_tag = VOL_TAG;
assign cur_tag = (range_sw == 2'd1) ? CUR_TAG1 :
                 (range_sw == 2'd2) ? CUR_TAG2 :
                 (range_sw == 2'd3) ? CUR_TAG3 :
                                      CUR_TAG0;



always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        wrfifo_wren <= 1'b0;
        wrfifo_din  <= 16'd0;
        state       <= ST_IDLE;
        wrfifo_clr  <= 1'b0;
        rst_cnt     <= 8'd0;
        valid       <= 1'b0;
    end else begin
        case(state)
            ST_IDLE: begin
                valid       <= 1'b0;
                wrfifo_wren <= 1'b0;
                wrfifo_clr  <= 1'b0;
                if(en) begin
                    wrfifo_clr <= 1'b1;   // 拉高fifo reset
                    rst_cnt    <= 8'd0;
                    state      <= ST_RST;
                end
            end

            ST_RST: begin
                // 保持reset一段时间
                wrfifo_clr <= 1'b1;
                rst_cnt    <= rst_cnt + 1'b1;
                if(rst_cnt >= 8'd15) begin // 等待 >=16 clk
                    wrfifo_clr <= 1'b0;
                    rst_cnt    <= 8'd0;
                    state      <= ST_WAIT;
                end
            end

            ST_WAIT: begin
                // 再空跑几个周期，保证FIFO内部稳定
                rst_cnt <= rst_cnt + 1'b1;
                if(rst_cnt >= 8'd15) begin
                    rst_cnt <= 8'd0;
                    state   <= ST_PREP;
                end
            end

            ST_PREP: begin
                valid       <= 1'b1;
                wrfifo_wren <= 1'b0;
                // en == 1 期间持续采样，en == 0 时停止采样
                if (en) begin
                    if(update_flag) begin
                        vol_buf <= {vol_tag, vol_samp};
                        cur_buf <= {cur_tag, cur_samp};
                        state <= ST_VHIGH;
                    end
                end else begin
                    state <= ST_IDLE;
                end
            end

            ST_VHIGH: begin
                wrfifo_wren <= 1'b1;
                wrfifo_din  <= vol_buf[31:16];
                state <= ST_VLOW;
            end

            ST_VLOW: begin
                wrfifo_wren <= 1'b1;
                wrfifo_din  <= vol_buf[15:0];
                state <= ST_CHIGH;
            end

            ST_CHIGH: begin
                wrfifo_wren <= 1'b1;
                wrfifo_din  <= cur_buf[31:16];
                state <= ST_CLOW;
            end

            ST_CLOW: begin
                wrfifo_wren <= 1'b1;
                wrfifo_din  <= cur_buf[15:0];
                state <= ST_PREP;   // 回到等待下一次 update_flag
            end

            default: state <= ST_IDLE;
        endcase
    end
end






endmodule
//*************** END OF LINE ********************//