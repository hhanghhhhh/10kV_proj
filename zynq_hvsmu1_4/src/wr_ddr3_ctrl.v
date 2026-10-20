/************************************************
//FILE NAME:        test_wr_ddr3.v
//DESCRIPTION:      test_wr_ddr3
//AUTHOR:           hehang
//DATE:             25/09/16
//LOG:              控制存储模块开、关，并计数
************************************************/
//************** Module Define *******************//
module wr_ddr3_ctrl
(   
    input                       sys_clk             ,  // 100M
    input                       sys_rst_n           ,
    input                       samp_update         ,
    input                       en                  ,
    input        [ 1:0]         range_sw            ,
    input        [ 7:0]         div                 ,
    input        [16:0]         total_num           ,
    input        [23:0]         samp_cur            ,
    input        [23:0]         samp_vol            ,
    output  reg                 ok                  ,
    output  wire                wrfifo_clr          ,
    output  wire                wrfifo_clk          ,
    output  wire [15:0]         wrfifo_din          ,
    output  wire                wrfifo_wren         
);

//*************** Parameter & Internal Register ****//
// parameter
localparam  DEBUG_FLAG = 0;     // 调试模式，模拟接收数据

// reg
reg  [ 7:0] div_cnt     ;


wire        valid       ;
wire [23:0] vol_samp    ;
wire [23:0] cur_samp    ;
reg  [23:0] test_vol    ;
reg  [23:0] test_cur    ;
wire [23:0] samp_cur_r  ;
wire [23:0] samp_vol_r  ;

reg  [ 7:0] clk_cnt     ;
wire        adc_update  ;
reg         test_update ;
reg         update_flag ;
reg  [16:0] stroed_num  ;
wire        samp_update_pedge;
//*************** Main Code ***********************//
// 模拟数据更新，0.5M
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        clk_cnt <= 8'd0;
        test_update <= 1'b0;
    end else begin
        if (clk_cnt == 8'd400 - 1'b1) begin
            clk_cnt <= 8'd0;
            test_update <= 1'b1;
        end else begin
            clk_cnt <= clk_cnt + 1'b1;
            test_update <= 1'b0;
        end
    end
end
// 模拟数据更新，1M
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        test_vol <= 24'd0;
        test_cur <= 24'd4194304;
    end else begin
        if (adc_update == 1'b1) begin
            test_vol <= test_vol + 24'd10000;
            test_cur <= test_cur + 24'd10000;
            // test_vol <= 24'd2000000;
            // test_cur <= 24'd4194304;
        end
    end
end

assign  adc_update = (DEBUG_FLAG == 1'b1) ? test_update : samp_update_pedge;
assign  vol_samp = (DEBUG_FLAG == 1'b1) ? test_vol : samp_vol_r;
assign  cur_samp = (DEBUG_FLAG == 1'b1) ? test_cur : samp_cur_r;


// 分频
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        div_cnt <= 8'd0;
    end else begin
        if (adc_update == 1'b1) begin
            if (div_cnt == div - 1'b1) begin
                div_cnt <= 8'd0;
            end else begin
                div_cnt <= div_cnt + 1'b1;
            end
        end
    end
end

// 储存
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        update_flag = 1'b0;
    end else begin
        if (adc_update == 1'b1 && div_cnt == div - 1'b1 &&
            stroed_num < total_num && en == 1'b1 && valid == 1'b1) begin
            update_flag <= 1'b1;
        end else begin
            update_flag <= 1'b0;
        end
    end
end

// 存储计数
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        stroed_num <= 17'd0;
    end else begin
        if (en == 1'b1) begin
            if (update_flag == 1'b1) begin
                stroed_num <= stroed_num + 1'b1;
            end
        end else begin
            stroed_num <= 17'd0;
        end
    end
end

// ok
always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        ok <= 1'b0;
    end else begin
        if (en == 1'b1 && stroed_num == total_num) begin
            ok <= 1'b1;
        end else begin
            ok <= 1'b0;
        end
    end
end


wr_samp_to_ddr3 wr_samp_to_ddr3_inst1 (
    .sys_clk            (sys_clk        ),
    .sys_rst_n          (sys_rst_n      ),
    .en                 (en             ),
    .vol_samp           (vol_samp       ),
    .cur_samp           (cur_samp       ),
    .update_flag        (update_flag    ),
    .range_sw           (range_sw       ),
    .valid              (valid          ),
    .wrfifo_clr         (wrfifo_clr     ),
    .wrfifo_clk         (wrfifo_clk     ),
    .wrfifo_din         (wrfifo_din     ),
    .wrfifo_wren        (wrfifo_wren    )
);


posedge_detect  samp_update_pedge_inst1
(
.sys_clk            (sys_clk            ),
.rst_n              (sys_rst_n          ),
.signal             (samp_update        ),
.signal_default     (1'b0               ),
.signal_pedge       (samp_update_pedge  )
);

async_double_flopping  #(
    .DATA_WIDTH      (24                )
)samp_cur_inst(         
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (samp_cur          ),
    .data_out        (samp_cur_r        )
);
async_double_flopping  #(
    .DATA_WIDTH      (24                )
)samp_vol_inst(         
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst_n         ),
    .data_in         (samp_vol          ),
    .data_out        (samp_vol_r        )      
);

endmodule
//*************** END OF LINE ********************//