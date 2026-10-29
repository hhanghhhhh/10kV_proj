/************************************************
//FILE NAME:        com_between_fpga_fast_rx.v
//DESCRIPTION:      com_between_fpga_fast_rx
//AUTHOR:           hehang
//DATE:             25/06/20
//LOG:              
************************************************/
//************** Module Define *******************//
module com_between_fpga_fast_rx(   
    input                       sys_clk             ,  // 200M
    input                       sys_rst_n           ,
    input                       rx_clk              ,
    input                       rx_cs_n             ,
    input                       rx_dataa            ,
    input                       rx_datab            ,

    output reg [23:0]           rx_vol_data         ,   // 接收到的电压数据
    output reg [23:0]           rx_cur_data         ,   // 接收到的电流数据
    output reg                  rx_done                 // 一帧接收完成标志
);

//*************** Parameter & Internal Register ****//
// parameter


// reg
reg     [23:0]      shift_vol       ;
reg     [23:0]      shift_cur       ;
reg     [ 4:0]      bit_cnt         ;
wire                rx_clk_negedge  ;
wire                rx_clk_r        ;
wire                rx_cs_n_r       ;
wire                rx_dataa_r      ;
wire                rx_datab_r      ;
reg     [ 4:0]      state           ;

//*************** Main Code ***********************//
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        shift_vol   <= 24'd0;
        shift_cur   <= 24'd0;
        rx_vol_data <= 24'd0;
        rx_cur_data <= 24'd0;
        bit_cnt     <= 5'd0;
        rx_done     <= 1'b0;
    end else begin
        if(rx_cs_n_r) begin
            // CS 拉高 -> 一帧结束
            if(bit_cnt == 24) begin
                // 已经完整收完 24bit
                rx_vol_data <= shift_vol;
                rx_cur_data <= shift_cur;
                rx_done     <= 1'b1;
                bit_cnt     <= 5'd0;
            end
            else begin
                bit_cnt     <= 5'd0;
                shift_vol   <= 24'd0;
                shift_cur   <= 24'd0;
            end
        end else begin
            rx_done     <= 1'b0;
            // CS 拉低 -> 接收中
            if(rx_clk_negedge) begin
                shift_vol <= {shift_vol[22:0], rx_dataa_r};
                shift_cur <= {shift_cur[22:0], rx_datab_r};
                bit_cnt <= bit_cnt + 1;
            end
        end
    end
end


negedge_detect  rx_negedge_inst1
(
.sys_clk            (sys_clk            ),
.rst_n              (sys_rst_n          ),
.signal             (rx_clk_r           ),
.signal_default     (1'b0               ),
.signal_nedge       (rx_clk_negedge     )
);

async_double_flopping  #(
    .DATA_WIDTH      (1                 )
)rx_clk_inst(           
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (rx_clk            ),
    .data_out        (rx_clk_r          )      
);

async_double_flopping  #(
    .DATA_WIDTH      (1                 )
)rx_cs_n_inst(          
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (rx_cs_n           ),
    .data_out        (rx_cs_n_r         )      
);

async_double_flopping  #(
    .DATA_WIDTH      (1                 )
)rx_dataa_inst(         
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (rx_dataa          ),
    .data_out        (rx_dataa_r        )      
);

async_double_flopping  #(
    .DATA_WIDTH      (1                 )
)rx_datab_inst(         
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (rx_datab          ),
    .data_out        (rx_datab_r        )      
);


endmodule
//*************** END OF LINE ********************//