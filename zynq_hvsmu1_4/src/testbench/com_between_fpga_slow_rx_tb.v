/************************************************
//FILE NAME:        com_between_fpga_slow_rx_tb.v
//DESCRIPTION:      com_between_fpga_slow_rx_tb
//AUTHOR:           hehang
//DATE:             25/06/22
//LOG:              
************************************************/

`timescale 1ns / 100ps
`define sys_clk_period_ns 5.0

module com_between_fpga_slow_rx_tb();

//reg
reg         sys_clk         ;
reg         sys_rst         ;

//sys_clk
initial sys_clk = 0;
always #(`sys_clk_period_ns / 2) sys_clk = ~sys_clk;

//sys_rst
initial begin
    sys_rst = 0;
    #100 sys_rst = 1;
end

wire    clk;
wire    data; 
reg     start;
wire    busy;
reg [15:0] cnt;

localparam  DATA_NUM = 10;
localparam  TXCLK_DIV_CNT = 4;

//Unit Instantiate
com_between_fpga_slow_tx 
#(
    .TXCLK_DIV_CNT       (TXCLK_DIV_CNT),
    .DATA_NUM            (DATA_NUM)
)tx_inst123
(   
    .sys_clk             (sys_clk),  // 200M
    .sys_rst_n           (sys_rst),
    .start               (start),
    .busy                (busy),
    .send_data1          (1),
    .send_data2          (2),
    .send_data3          (3),
    .send_data4          (4),
    .send_data5          (5),
    .send_data6          (6),
    .send_data7          (7),
    .send_data8          (8),
    .send_data9          (9),
    .send_data10         (10),
    .tx_clk              (clk),
    .tx_data             (data)
);

// 接收间隔 3 个 tx_clk
com_between_fpga_slow_rx
#(
    .OVERITME_CNT        (TXCLK_DIV_CNT * (5 - 2)),
    .DATA_NUM            (DATA_NUM)      
) rx_inst123
(   
    .sys_clk             (sys_clk),  // 200M
    .sys_rst_n           (sys_rst),
    .rx_clk              (clk),  // 对端发来的 clk
    .rx_data             (data),  // 对端发来的 data
    .valid               (),  // 数据接收成功标志
    .rx_data1            (),
    .rx_data2            (),
    .rx_data3            (),
    .rx_data4            (),
    .rx_data5            (),
    .rx_data6            (),
    .rx_data7            (),
    .rx_data8            (),
    .rx_data9            (),
    .rx_data10           () 
);



//Stimulus process
initial begin
    
end

// 间隔 5 个 tx_clk 发送一次
always@(posedge sys_clk or negedge sys_rst) begin
    if (!sys_rst) begin
        cnt <= 16'd0;
    end else begin
        if (busy == 1'b0) begin
            cnt <= cnt + 1'b1;
        end else begin
            cnt <= 16'd0;
        end
    end
end

always@(posedge sys_clk or negedge sys_rst) begin
    if (!sys_rst) begin
        start <= 0;
    end else begin
        if (cnt > (TXCLK_DIV_CNT * 5)) begin
            start <= 1;
        end else begin
            start <= 0;
        end
    end
end



endmodule