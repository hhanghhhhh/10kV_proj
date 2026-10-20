//************************************************//
//FILE NAME:        TOP.V
//DESCRIPTION:
//AUTHOR:           hehang
//DATE:             20230724
//LOG:
//************************************************//

//************** Module Define *******************//
module TOP
(
    input   clk_external                                , //晶振时钟，25M


    // idac - dac1 
    output                      idac1_cs                ,
    output                      idac1_sclk              ,
    output                      idac1_sdi               ,
    output                      idac_slowen             ,
    output                      idac_slow_a0            ,
    output                      idac_slow_a1            ,
    // vdac - dac2
    output                      vdac2_cs                ,
    output                      vdac2_sclk              ,
    output                      vdac2_sdi               ,
    output                      vdac_slowen             ,
    output                      vdac_slow_a0            ,
    output                      vdac_slow_a1            ,
    // ADC
    output                      adc_rstn                ,
    input                       adc_busy                ,
    output                      adc_csn                 ,
    output                      adc_cnv                 ,
    output                      adc_sck                 ,
    output                      adc_sdi                 ,
    input                       adc_sdo0                ,
    input                       adc_sdo1                ,
    input                       adc_sdo2                ,
    input                       adc_sdo3                ,
    input                       adc_sdo4                ,
    input                       adc_sdo5                ,
    input                       adc_sdo6                ,
    input                       adc_sdo7                ,



    // 两片 fpga 之间的通信
    input  [11:0]               com_in                  ,
    output [11:0]               com_out                 ,

    // signal
    output                      vma0                    ,
    output                      vma1                    ,
    output                      vma2                    ,
    output                      vma_en                  ,
    output                      va0                     ,
    output                      va1                     ,
    output                      va2                     ,
    output                      va_en                   ,
    output                      idac_amps               ,
    output                      imeas_gain10            ,
    output                      imeas_gain1             ,
    output                      ifb_gain3               ,
    output                      ifb_gain1               ,
    output                      cmode_n                 ,
    output                      cmode_bw1_n             ,
    output                      cmode_bw2_n             ,
    output                      cmult23_n               ,
    output                      range_10nA_100nA        ,
    output                      range_1uA_10uA          ,
    output                      range_100uA_1mA         ,
    output                      range_5mA_50mA          ,
    output                      lo_protect              ,
    output                      outoff                  ,
    output                      pramp_p                 ,
    output [4:2]                reserved                ,
    output                      realy_vo                ,
    input                       in_over_temp            
);

//*************** Parameter & Internal Register ****//
localparam  FPGA_VERSION      = 2;
localparam  RUN_FLAG          = 10;
localparam  COM_FPGA_SLOW_DATA_NUM      = 10;
localparam  COM_FPGA_SLOW_TXCLK_DIV_CNT = 4;

wire                            sys_clk_200m;
wire                            sys_clk_50m; 
wire                            sys_clk_150m; 
wire                            sys_clk_25m;
wire                            sys_clk;
wire                            sys_rst;
wire                            sys_rst_pll;
wire                            sys_rst_dsp;

reg                             slow_tx_start;
wire                            slow_tx_busy;
reg  [15:0]                     slow_tx_interval_cnt;
wire                            slow_tx_clk;
wire                            slow_tx_data;
wire                            slow_rx_clk;
wire                            slow_rx_data;

wire                            fast_tx_clk;
wire                            fast_tx_cs_n;
wire                            fast_tx_dataa;
wire                            fast_tx_datab;


wire                            dataok;
wire                            dataok_cur;
wire                            dataok_vol;
wire                            adc_sample_en_wire;
wire [23:0]                     adc_dataa_vol;
wire [23:0]                     adc_datab_cur;
wire [23:0]                     adc_dataa_vol_filter;
wire [23:0]                     adc_datab_cur_filter;
wire [23:0]                     adc_dataa_vol_send;
wire [23:0]                     adc_datab_cur_send;
wire                            adc_cnv_flag_send;

wire [31:0]                     slowrx_cmd;     //synthesis keep
wire [31:0]                     slowrx_cmd2;    //synthesis keep
wire [31:0]                     slowrx_dacset;      //synthesis keep
wire [31:0]                     slowrx_samppara;    //synthesis keep
wire [31:0]                     slowtx_state;
wire [31:0]                     slowtx_vol_avg;
wire [31:0]                     slowtx_cur_avg;
wire [31:0]                     slowtx_vol;
wire [31:0]                     slowtx_cur;

wire [15:0]                     vdac_set;
wire [15:0]                     idac_set;
wire [15:0]                     samp_num;
wire [15:0]                     intervla_cnt;
wire                            samp_start;
wire [23:0]                     volsamp_avg;
wire [23:0]                     cursamp_avg;

wire                            relay;
wire                            wave_filter_en;
//*************** Main Code ***********************//
assign      sys_rst = sys_rst_pll;
assign      sys_clk = sys_clk_25m;

assign      slow_rx_clk = com_in[0];
assign      slow_rx_data = com_in[1];
assign      com_out[0]  =   slow_tx_clk;
assign      com_out[1]  =   slow_tx_data;

assign      com_out[2]           = fast_tx_clk   ;
assign      com_out[3]           = fast_tx_cs_n  ;
assign      com_out[4]           = fast_tx_dataa ;   // 电压 vol
assign      com_out[5]           = fast_tx_datab ;   // 电流 cur

assign      dataok = dataok_cur && dataok_vol;
assign      realy_vo = relay;


assign      slowtx_state[0]     = dataok;
assign      slowtx_state[8:1]   = FPGA_VERSION;     // 8bit
assign      slowtx_state[12:9]  = RUN_FLAG;         // 4bit
assign      slowtx_state[13]    = in_over_temp; 
assign      slowtx_vol_avg      = {{8{volsamp_avg[23]}}, volsamp_avg};
assign      slowtx_cur_avg      = {{8{cursamp_avg[23]}}, cursamp_avg};
assign      slowtx_vol          = {{8{adc_dataa_vol[23]}}, adc_dataa_vol};
assign      slowtx_cur          = {{8{adc_datab_cur[23]}}, adc_datab_cur};

assign      vdac_set            = slowrx_dacset[15:0];
assign      idac_set            = slowrx_dacset[31:16];
assign      samp_num            = slowrx_samppara[15:0];
assign      intervla_cnt        = slowrx_samppara[31:16];
assign      samp_start          = slowrx_cmd[0];
assign      vdac_slowen         = slowrx_cmd[1];
assign      vdac_slow_a0        = slowrx_cmd[2];
assign      vdac_slow_a1        = slowrx_cmd[3];
assign      idac_amps           = slowrx_cmd[4];
assign      idac_slowen         = slowrx_cmd[5];
assign      idac_slow_a0        = slowrx_cmd[6];
assign      idac_slow_a1        = slowrx_cmd[7];
assign      vma2                = slowrx_cmd2[0];
assign      va2                 = slowrx_cmd2[1];
assign      vma0                = slowrx_cmd[8];
assign      vma1                = slowrx_cmd[9];
assign      vma_en              = slowrx_cmd[10];
assign      va0                 = slowrx_cmd[11];
assign      va1                 = slowrx_cmd[12];
assign      va_en               = slowrx_cmd[13];
assign      imeas_gain10        = slowrx_cmd[14];
assign      imeas_gain1         = slowrx_cmd[15];
assign      ifb_gain3           = slowrx_cmd[16];
assign      ifb_gain1           = slowrx_cmd[17];
assign      cmode_n             = slowrx_cmd[18];
assign      cmode_bw1_n         = slowrx_cmd[19];
assign      cmode_bw2_n         = slowrx_cmd[20];
assign      cmult23_n           = slowrx_cmd[21];
assign      range_10nA_100nA    = ~slowrx_cmd[22];
assign      range_1uA_10uA      = ~slowrx_cmd[23];
assign      range_100uA_1mA     = ~slowrx_cmd[24];
assign      range_5mA_50mA      = ~slowrx_cmd[25];
assign      lo_protect          = slowrx_cmd[26];
assign      outoff              = slowrx_cmd[27];
assign      pramp_p             = slowrx_cmd[28];
assign      relay               = slowrx_cmd[29];
assign      wave_filter_en      = slowrx_cmd[30];




// PLL
ADC_sys_clk_PLL pll_inst1
(
    .refclk                 (clk_external       ),
    .reset                  (1'b0               ),
    .lock                   (sys_rst_pll        ),  //时钟稳定后输出1，该信号作为整体的复位信号
    .clk0_out               (sys_clk_200m       ),
    .clk1_out               (sys_clk_25m        ),  //25M,系统时钟
    .clk2_out               (sys_clk_50m        ),  //50M
    .clk3_out               (sys_clk_150m       )   //150M
);



// 接收间隔 3 个 tx_clk
com_between_fpga_slow_rx #(
    .OVERITME_CNT        (COM_FPGA_SLOW_TXCLK_DIV_CNT * (5 - 2)),
    .DATA_NUM            (COM_FPGA_SLOW_DATA_NUM)      
) slow_rx_inst
(   
    .sys_clk             (sys_clk_200m      ),  // 200M
    .sys_rst_n           (sys_rst           ),
    .rx_clk              (slow_rx_clk       ),  // 对端发来的 clk
    .rx_data             (slow_rx_data      ),  // 对端发来的 data
    .valid               (                  ),  // 数据接收成功标志
    .rx_data1            (slowrx_cmd        ),
    .rx_data2            (slowrx_dacset     ),
    .rx_data3            (slowrx_samppara   ),
    .rx_data4            (slowrx_cmd2       ),
    .rx_data5            (                  ),
    .rx_data6            (                  ),
    .rx_data7            (                  ),
    .rx_data8            (                  ),
    .rx_data9            (                  ),
    .rx_data10           (                  ) 
);



wire adc_cnv_negedge;
wire adc_filter_valid;
negedge_detect adc_cnv_negedge_inst(
    .sys_clk            (sys_clk_50m        ),
    .rst_n              (sys_rst            ),
    .signal             (adc_cnv            ),
    .signal_default     (1'b0               ),
    .signal_nedge       (adc_cnv_negedge    )
);

median_filter #(
    .DATA_WIDTH (24)
) median_filter_vol_inst (
    .sys_clk             (sys_clk_50m       ),
    .sys_rst_n           (sys_rst           ),
    .din                 (adc_dataa_vol     ),
    .din_valid           (adc_cnv_negedge   ),
    .dout                (adc_dataa_vol_filter),
    .dout_valid          (adc_filter_valid  )
);
median_filter #(
    .DATA_WIDTH (24)
) median_filter_cur_inst (
    .sys_clk             (sys_clk_50m       ),
    .sys_rst_n           (sys_rst           ),
    .din                 (adc_datab_cur     ),
    .din_valid           (adc_cnv_negedge   ),
    .dout                (adc_datab_cur_filter),
    .dout_valid          (                  )
);

assign   adc_cnv_flag_send = (wave_filter_en) ? adc_filter_valid : adc_cnv;
assign   adc_dataa_vol_send = (wave_filter_en) ? adc_dataa_vol_filter : adc_dataa_vol;
assign   adc_datab_cur_send = (wave_filter_en) ? adc_datab_cur_filter : adc_datab_cur;

com_between_fpga_fast_tx fast_tx_inst (
    .sys_clk             (sys_clk_200m      ),
    .sys_rst_n           (sys_rst           ),
    .cnv_flag            (adc_cnv_flag_send ),
    .vol_samp            (adc_dataa_vol_send),
    .cur_samp            (adc_datab_cur_send),
    .tx_clk              (fast_tx_clk       ),
    .tx_cs_n             (fast_tx_cs_n      ),
    .tx_dataa            (fast_tx_dataa     ),
    .tx_datab            (fast_tx_datab     )
);


// 向另一个 fpga 发送数据的慢速模块
// 间隔 5 个 tx_clk 发送一次
always@(posedge sys_clk_200m or negedge sys_rst) begin
    if (!sys_rst) begin
        slow_tx_interval_cnt <= 16'd0;
    end else begin
        if (slow_tx_busy == 1'b0) begin
            slow_tx_interval_cnt <= slow_tx_interval_cnt + 1'b1;
        end else begin
            slow_tx_interval_cnt <= 16'd0;
        end
    end
end
always@(posedge sys_clk_200m or negedge sys_rst) begin
    if (!sys_rst) begin
        slow_tx_start <= 0;
    end else begin
        if (slow_tx_interval_cnt > (COM_FPGA_SLOW_TXCLK_DIV_CNT * 5)) begin
            slow_tx_start <= 1;
        end else begin
            slow_tx_start <= 0;
        end
    end
end

com_between_fpga_slow_tx #(
    .TXCLK_DIV_CNT       (COM_FPGA_SLOW_TXCLK_DIV_CNT),
    .DATA_NUM            (COM_FPGA_SLOW_DATA_NUM)
)slow_tx_inst1
(   
    .sys_clk             (sys_clk_200m      ),  // 200M
    .sys_rst_n           (sys_rst           ),
    .start               (slow_tx_start     ),
    .busy                (slow_tx_busy      ),
    .send_data1          (slowtx_vol_avg    ),
    .send_data2          (slowtx_cur_avg    ),
    .send_data3          (slowtx_vol        ),
    .send_data4          (slowtx_cur        ),
    .send_data5          (slowtx_state      ),
    .send_data6          (6                 ),
    .send_data7          (7                 ),
    .send_data8          (8                 ),
    .send_data9          (9                 ),
    .send_data10         (10                ),
    .tx_clk              (slow_tx_clk       ),
    .tx_data             (slow_tx_data      )
);


//DAC
DAC_CTRL  vdac_ctrl_inst1
(   
    .sys_clk    (sys_clk                ),  //系统时钟  25M
    .sys_rst    (sys_rst                ),
    .DAC_Set    (vdac_set               ),
    .cs_n       (vdac2_cs               ),
    .sclk       (vdac2_sclk             ),
    .mosi       (vdac2_sdi              ),
    .Ok_Flag    (                       ) 
);

DAC_CTRL idac_ctrl_inst1
(   
    .sys_clk    (sys_clk                ),  //系统时钟  25M
    .sys_rst    (sys_rst                ),
    .DAC_Set    (idac_set               ),
    .cs_n       (idac1_cs               ),
    .sclk       (idac1_sclk             ),
    .mosi       (idac1_sdi              ),
    .Ok_Flag    (                       ) 
);


//高精度24bitADC
ADC_24_RST  adc24_rstl1
(   
    .sys_clk        (sys_clk            ),  //系统时钟  25M
    .sys_rst        (sys_rst            ),  //复位信号，低有效
    .en             (1'b1               ),  //模块使能信号，高有效
    .rst_n          (adc_rstn           ),  //
    .adc_sample_en  (adc_sample_en_wire )   //开始转换
);

ADC_24_DRV adc24_drv_1
(   
    .sys_clk        (sys_clk_200m       ),  //时钟  200M
    .sys_rst        (sys_rst            ),  //复位信号，低有效
    .en             (adc_sample_en_wire ),  //模块使能信号，高有效
    .dataA          (adc_dataa_vol      ),  //读取到的24bitADC数据
    .dataB          (adc_datab_cur      ),
    .cs_n           (adc_csn            ),
    .cnv            (adc_cnv            ),
    .sck            (adc_sck            ),
    .misoA          (adc_sdo0           ),
    .misoB          (adc_sdo4           )
);


// 求平均值
ADC_DATA_AVG_DIV #(
    .DATA_WIDTH(24),   
    .ADD_WIDTH(40)
)data_vol_avg_inst
(   
    .sys_clk        (sys_clk                    ),  //系统时钟  25M
    .sys_rst        (sys_rst                    ),  //复位信号，低有效
    .en             (samp_start                 ),  //上升沿触发采样
    .sample_num     (samp_num                   ),  //采样点数
    .interval_cnt   (intervla_cnt               ),  //间隔多久取一个点,最少=50，2us， 最大65535
    .adc_data       (adc_dataa_vol              ),  //采样值
    .data_ok        (dataok_vol                 ),  //
    .data_avg_out   (volsamp_avg                ),
    .data_add_out   (                           )   //输出总和
);


ADC_DATA_AVG_DIV #(
    .DATA_WIDTH(24),   
    .ADD_WIDTH(40)
)data_cur_avg_inst
(   
    .sys_clk        (sys_clk                    ),  //系统时钟  25M
    .sys_rst        (sys_rst                    ),  //复位信号，低有效
    .en             (samp_start                 ),  //上升沿触发采样
    .sample_num     (samp_num                   ),  //采样点数
    .interval_cnt   (intervla_cnt               ),  //间隔多久取一个点,最少=50，2us， 最大65535
    .adc_data       (adc_datab_cur              ),  //采样值
    .data_ok        (dataok_cur                 ),  //
    .data_avg_out   (cursamp_avg                ),
    .data_add_out   (                           )   //输出总和
);





endmodule


//*************** END OF LINE ********************//

