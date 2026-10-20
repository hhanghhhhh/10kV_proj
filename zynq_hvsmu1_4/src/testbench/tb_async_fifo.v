/************************************************
//FILE NAME:        tb_async_fifo.v
//DESCRIPTION:      tb_async_fifo
//AUTHOR:           hehang
//DATE:             25/10/27
//LOG:              
************************************************/

`timescale 1ns / 100ps
`define sys_clk_period_ns 10.0

module tb_async_fifo();

//reg
reg         sys_clk         ;
reg         sys_rst         ;
reg         clk_50M         ;

//sys_clk
initial sys_clk = 0;
always #(`sys_clk_period_ns / 4) sys_clk = ~sys_clk;

initial clk_50M = 0;
always #(`sys_clk_period_ns) clk_50M = ~clk_50M;

//sys_rst
initial begin
    sys_rst = 0;
    #100 sys_rst = 1;
end


//Unit Instantiate


//Stimulus process
reg [31:0]  test_data;


initial begin
    test_data = 0;
end

always@(posedge sys_clk) begin
    test_data <= test_data + 1;
end



async_fifo  #(
	.DATA_WIDTH                     (32         )			//FIFO位宽
)lowrx_err_cnt_fifo_inst(				
    .rst_n                          (sys_rst    ),
    .wr_clk		                    (sys_clk    ), //写时钟
    .data_in	                    (test_data  ), //写入的数据
    .rd_clk		                    (clk_50M    ), //读时钟					                                        
    .data_out	                    (           )  //输出的数据
);


endmodule