/************************************************
//FILE NAME:        com_between_fpga_slow_tx.v
//DESCRIPTION:      com_between_fpga_slow_tx
//AUTHOR:           hehang
//DATE:             25/06/20
//LOG:              串行发送数据，额外加头、尾、累加和校验
//                  发送时间计算：200M = 5ns，n个U32 共32*n bit，一个bit的时间为 5ns * DIV_CNT
//                  因此发送时间为： 5 * 32 * n * DIV_CNT
//                  另外，每帧需要有发送间隔，设置为 10 个 tx_clk 时间 = 10 * DIV_CNT * 5
//                  TXCLK_DIV_CNT 设置8时，传10个数据，共13个U32，需要17.04s （5 * 32 * 8 * 13 + 10 * 8 * 5）
//                  TXCLK_DIV_CNT 设置4时，传10个数据，共13个U32，需要8.52us
//                  TXCLK_DIV_CNT 设置2时，传10个数据，共13个U32，需要4.26us (TXCLK_DIV_CNT 最小设置为 2)
************************************************/
//************** Module Define *******************//
module com_between_fpga_slow_tx
#(
    parameter   TXCLK_DIV_CNT   =       8       ,
    parameter   DATA_NUM        =       2       
)
(   
    input                       sys_clk             ,  // 200M
    input                       sys_rst_n           ,
    input                       start               ,
    input           [31:0]      send_data1          ,
    input           [31:0]      send_data2          ,
    input           [31:0]      send_data3          ,
    input           [31:0]      send_data4          ,
    input           [31:0]      send_data5          ,
    input           [31:0]      send_data6          ,
    input           [31:0]      send_data7          ,
    input           [31:0]      send_data8          ,
    input           [31:0]      send_data9          ,
    input           [31:0]      send_data10         ,
    output  reg                 busy                ,
    output  wire                tx_clk              ,
    output  reg                 tx_data             
);

//*************** Parameter & Internal Register ****//
// parameter
localparam  TXCLK_DIV_CNT_DIV2 = TXCLK_DIV_CNT >> 1;
localparam  FRAME_HEAD        = 32'h5555AAAA;
localparam  FRAME_TAIL        = 32'hAAAA5555;
localparam  TOTAL_U32S        = DATA_NUM + 3;

localparam  S_IDEL              = 4'b0000;
localparam  S_LOAD_BUFFER       = 4'b0001;
localparam  S_SEND_DATA_FIRST   = 4'b0010;
localparam  S_SEND_DATA         = 4'b0100;
localparam  S_SEND_DATA_LAST    = 4'b1000;

// reg
reg     [3:0]   state           ;
reg     [7:0]   buf_index       ;
reg     [7:0]   bit_index       ;
reg             tx_clk_tmp      ;
reg     [7:0]   tx_clk_cnt      ;
reg             tx_clk_tmp_r    ;
reg             tx_clk_en       ;
reg             tx_clk_nedge    ;
reg     [31:0]  buffer [0:TOTAL_U32S - 1];
reg     [31:0]  send_word       ;
reg     [31:0]  checksum        ;
//*************** Main Code ***********************//
// tx_clk_tmp
always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        tx_clk_cnt <= 8'd0;
        tx_clk_nedge <= 1'b0;
        tx_clk_tmp <= 1'b0;
    end else begin
        if (tx_clk_en) begin
            if (tx_clk_cnt < TXCLK_DIV_CNT - 1'b1) begin
                tx_clk_cnt <= tx_clk_cnt + 1'b1;
                tx_clk_nedge <= 1'b0;
                if (tx_clk_cnt == TXCLK_DIV_CNT_DIV2 - 1'b1)
                    tx_clk_tmp <= 1'b1;
            end else begin
                tx_clk_cnt <= 8'd0;
                tx_clk_tmp <= 1'b0;
                tx_clk_nedge <= 1'b1;
            end
        end else begin
            tx_clk_cnt <= 8'd0;
            tx_clk_nedge <= 1'b0;
            tx_clk_tmp <= 1'b0;
        end
    end
end

always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        tx_clk_tmp_r <= 1'b0;
    end else begin
        tx_clk_tmp_r <= tx_clk_tmp;
    end
end

// 将 tx_clk 延迟1个周期，保证下降沿和数据变化同步
assign  tx_clk = tx_clk_tmp_r;



always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        state       <= S_IDEL;
        buf_index   <= 0;
        bit_index   <= 0;
        tx_data     <= 1'b0;
        tx_clk_en   <= 1'b0;
        busy        <= 1'b0;
        send_word   <= 32'd0;
        checksum    <= 32'd0;
    end else begin
        case (state)
            S_IDEL: begin
                tx_clk_en <= 1'b0;
                if (start) begin
                    state <= S_LOAD_BUFFER;
                    busy  <= 1'b1;
                end
            end
            S_LOAD_BUFFER: begin
                // 准备数据帧
                buffer[0] <= FRAME_HEAD;
                // ---暂时预留10个数据---
                if (DATA_NUM > 0)   buffer[1]  <= send_data1 ;
                if (DATA_NUM > 1)   buffer[2]  <= send_data2 ;
                if (DATA_NUM > 2)   buffer[3]  <= send_data3 ;
                if (DATA_NUM > 3)   buffer[4]  <= send_data4 ;
                if (DATA_NUM > 4)   buffer[5]  <= send_data5 ;
                if (DATA_NUM > 5)   buffer[6]  <= send_data6 ;
                if (DATA_NUM > 6)   buffer[7]  <= send_data7 ;
                if (DATA_NUM > 7)   buffer[8]  <= send_data8 ;
                if (DATA_NUM > 8)   buffer[9]  <= send_data9 ;
                if (DATA_NUM > 9)   buffer[10] <= send_data10;
                // ---暂时预留10个数据---
                buffer[DATA_NUM + 1] <= 32'd0;
                buffer[DATA_NUM + 2] <= FRAME_TAIL;

                bit_index  <= 0;
                buf_index  <= 0;
                state      <= S_SEND_DATA_FIRST;
            end
            S_SEND_DATA_FIRST: begin
                send_word  <= buffer[buf_index];
                checksum   <= buffer[buf_index];

                // 在第一个上升沿前给 tx_data 第一个数据
                tx_data    <= buffer[buf_index][31 - bit_index];
                bit_index  <= bit_index + 1;
                // 开始 clk
                tx_clk_en <= 1'b1;
                // 开始发送数据
                state <= S_SEND_DATA;
            end
            S_SEND_DATA: begin
                if (tx_clk_nedge) begin
                    // 发送当前 bit
                    tx_data <= send_word[31 - bit_index];

                    // 更新累加和
                    if (bit_index == 0 && buf_index < DATA_NUM + 1) begin
                        checksum <= checksum + send_word;
                    end

                    // 累加和计算完成后写入累加和
                    if (bit_index == 1 && buf_index == DATA_NUM) begin
                        buffer[DATA_NUM + 1] <= checksum;
                    end

                    // 更新 bit_index / buf_index
                    if (bit_index == 31) begin
                        bit_index <= 0;
                        
                        // 发送完成
                        if (buf_index == TOTAL_U32S - 1) begin
                            state <= S_SEND_DATA_LAST;
                            buf_index <= 0;
                        end else begin
                            send_word <= buffer[buf_index + 1'b1];
                            buf_index <= buf_index + 1'b1;
                        end
                    end else begin
                        bit_index <= bit_index + 1;
                    end
                end
            end
            S_SEND_DATA_LAST: begin
                // 最后一个数据发送后，多生成一个clk用于对端上升沿接收
                if (tx_clk_nedge) begin
                    busy <= 1'b0;
                    tx_clk_en <= 1'b0;
                    tx_data   <= 1'b0;
                    state     <= S_IDEL;
                end
            end
            default: state <= S_IDEL;
        endcase
    end
end








endmodule
//*************** END OF LINE ********************//