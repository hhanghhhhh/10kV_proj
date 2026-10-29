module top
(
    // SYS 
    input              external_clk            ,    // 50M
    // input             external_reset_n        ,
	// zynq_PS
    inout  [14:0]      DDR_addr                ,
    inout  [2:0]       DDR_ba                  ,
    inout              DDR_cas_n               ,
    inout              DDR_ck_n                ,
    inout              DDR_ck_p                ,
    inout              DDR_cke                 ,
    inout              DDR_cs_n                ,
    inout  [3:0]       DDR_dm                  ,
    inout  [31:0]      DDR_dq                  ,
    inout  [3:0]       DDR_dqs_n               ,
    inout  [3:0]       DDR_dqs_p               ,
    inout              DDR_odt                 ,
    inout              DDR_ras_n               ,    
    inout              DDR_reset_n             ,
    inout              DDR_we_n                ,
    inout              FIXED_IO_ddr_vrn        ,
    inout              FIXED_IO_ddr_vrp        ,
    inout  [53:0]      FIXED_IO_mio            ,
    inout              FIXED_IO_ps_clk         ,
    inout              FIXED_IO_ps_porb        ,
    inout              FIXED_IO_ps_srstb       ,
    inout              led1                    ,
    inout              led2                    ,
    inout              trigger_in              ,
    inout              trigger_out             ,
    inout              pfc_en                  ,
    inout              pfc_drive               ,
    inout              pfc_ok                  ,
    inout              pfc_rvd                 ,
    input  [11:0]      com_in                  ,
    output [11:0]      com_out                 
);

localparam  CSR_ADDR_SIZE               = 12;

localparam  COM_FPGA_SLOW_DATA_NUM      = 10;
localparam  COM_FPGA_SLOW_TXCLK_DIV_CNT = 4;

//PL使用DDR的基地址，留一定空间给PS用
localparam  DDR_BASE_ADDR = 32'h6000000; // 从DDR的第100M地址开始
localparam  DDR_LENTH = 32'h100000; // 1M

wire                            external_reset_n;      
wire                            ps2pl_clk50m_0;
wire                            ps2pl_clk100m_0;
wire                            ps2pl_resetn_0;
wire                            reset_pre;
reg  [19:0]                     reset_sync;
wire                            reset;
wire                            pll_reset_n;
wire                            pll_locked;
wire                            pll_clk_10m;
wire                            pll_clk_25m           ;
wire                            pll_clk_50m;
wire                            pll_clk_200m;
wire                            pll_clk_100m          ;
wire                            pl_reset_n;
wire                            LIT_memwr_en;
wire [CSR_ADDR_SIZE-1:0]        LIT_memwr_addr;
wire [31:0]                     LIT_memwr_dat;
wire [3:0]                      LIT_memwr_strb;
wire                            LIT_memrd_en;
wire [CSR_ADDR_SIZE-1:0]        LIT_memrd_addr;
wire                            REG_itf_wr_done;
wire                            REG_bresp_slverr;
wire                            REG_itf_rd_done;
wire                            REG_rresp_slverr;
wire [31:0]                     REG_itf_rdata;

wire [7:0]                      GPIO_0_0_tri_io;

reg                             slow_tx_start;
wire                            slow_tx_busy;
reg  [15:0]                     slow_tx_interval_cnt;
wire                            slow_tx_clk;
wire                            slow_tx_data;
wire                            slow_rx_clk;
wire                            slow_rx_data;

wire [31:0]                     slowtx_cmd;
wire [31:0]                     slowtx_cmd2;
wire [31:0]                     slowtx_dacset;
wire [31:0]                     slowtx_samppara;
wire [31:0]                     slowrx_state;
wire [31:0]                     slowrx_vol_avg;
wire [31:0]                     slowrx_cur_avg;
wire [31:0]                     slowrx_vol;
wire [31:0]                     slowrx_cur;
wire [31:0]                     lowrx_err_cnt;
wire [31:0]                     ps_to_pl_cmd         ;
wire [31:0]                     pl_to_ps_data        ;
wire [31:0]                     ps_to_pl_cmd2        ;
wire [31:0]                     pl_to_ps_data2       ;
wire [31:0]                     ps_to_pl_test_ddr3   ;
wire [31:0]                     pl_to_ps_test_ddr3   ;
wire [31:0]                     ps_to_pl_wave_para   ;
wire [15:0]                     test_wr_fifo_cnt     ;
wire [15:0]                     test_rd_fifo_cnt     ;

// fast
(* MARK_DEBUG = "true" *) wire                            fast_rx_clk          ;
(* MARK_DEBUG = "true" *) wire                            fast_rx_cs_n         ;
(* MARK_DEBUG = "true" *) wire                            fast_rx_dataa        ;
(* MARK_DEBUG = "true" *) wire                            fast_rx_datab        ;
wire [23:0]                     fast_rx_vol_data     ;
wire [23:0]                     fast_rx_cur_data     ;
wire                            fast_rx_done         ;




//wr_fifo Interface
wire                            wrfifo_clr          ;
wire                            wrfifo_clk          ;
wire [15:0]                     wrfifo_din          ;
wire                            wrfifo_wren         ;
//rd_fifo Interface
wire                            rdfifo_clr          ;
wire                            rdfifo_clk          ;
wire                            rdfifo_rden         ;
wire [15 :0]                    rdfifo_dout         ;
//axi                   
wire [3:0]                      s_axi_awid          ;
wire [31:0]                     s_axi_awaddr        ;
wire [7:0]                      s_axi_awlen         ;
wire [2:0]                      s_axi_awsize        ;
wire [1:0]                      s_axi_awburst       ;
wire [0:0]                      s_axi_awlock        ;
wire [3:0]                      s_axi_awcache       ;
wire [2:0]                      s_axi_awprot        ;
wire [3:0]                      s_axi_awqos         ;
wire [3:0]                      s_axi_awregion      ;
wire                            s_axi_awvalid       ;
wire                            s_axi_awready       ;
//                  
wire [63:0]                     s_axi_wdata         ;
wire [7:0]                      s_axi_wstrb         ;
wire                            s_axi_wlast         ;
wire                            s_axi_wvalid        ;
wire                            s_axi_wready        ;
//                  
wire [3:0]                      s_axi_bid           ;
wire [1:0]                      s_axi_bresp         ;
wire                            s_axi_bvalid        ;
wire                            s_axi_bready        ;
//                  
wire [3:0]                      s_axi_arid          ;
wire [31:0]                     s_axi_araddr        ;
wire [7:0]                      s_axi_arlen         ;
wire [2:0]                      s_axi_arsize        ;
wire [1:0]                      s_axi_arburst       ;
wire [0:0]                      s_axi_arlock        ;
wire [3:0]                      s_axi_arcache       ;
wire [2:0]                      s_axi_arprot        ;
wire [3:0]                      s_axi_arqos         ;
wire [3:0]                      s_axi_arregion      ;
wire                            s_axi_arvalid       ;
wire                            s_axi_arready       ;
//                  
wire [3:0]                      s_axi_rid           ;
wire [63:0]                     s_axi_rdata         ;
wire [1:0]                      s_axi_rresp         ;
wire                            s_axi_rlast         ;
wire                            s_axi_rvalid        ;
wire                            s_axi_rready        ;
//                  
wire                            s_axi_aclk          ;
wire                            s_axi_resetn        ;


// wave
wire [16 :0]                    wave_num            ;
wire [ 7 :0]                    wave_div            ;
wire                            wave_ok             ;


assign      s_axi_aclk                  = pll_clk_100m              ;
assign      s_axi_resetn                = pl_reset_n                ;

assign      pl_to_ps_data2[0]          = wave_ok                   ;
 
assign      slow_rx_clk = com_in[0];
assign      slow_rx_data = com_in[1];
assign      com_out[0]  =   slow_tx_clk;
assign      com_out[1]  =   slow_tx_data;
assign      fast_rx_clk                 = com_in[2]                 ;
assign      fast_rx_cs_n                = com_in[3]                 ;
assign      fast_rx_dataa               = com_in[4]                 ;
assign      fast_rx_datab               = com_in[5]                 ;


assign      wave_num                    = ps_to_pl_wave_para[16:0]  ;
assign      wave_div                    = ps_to_pl_wave_para[24:17] ;




assign      external_reset_n = 1'b1;        // 暂无外部 reset 信号
// PS 复位和外部复位信号共同给PLL
// PLL 稳定后的 locked 信号延时 20clk 作为 PL 模块的复位信号
assign      pll_reset_n = ps2pl_resetn_0 & external_reset_n;
assign      reset_pre = ~pll_locked;

always@(posedge pll_clk_200m or posedge reset_pre)
begin
    if (reset_pre) begin
        reset_sync <= {20{1'b1}};
    end else begin
        reset_sync <= reset_sync << 1;
    end
end
assign pl_reset_n = ~reset_sync[19];


assign  led1        = GPIO_0_0_tri_io[0];
assign  led2        = GPIO_0_0_tri_io[1];
assign  trigger_in  = GPIO_0_0_tri_io[2];
assign  trigger_out = GPIO_0_0_tri_io[3];
assign  pfc_en      = GPIO_0_0_tri_io[4];
assign  pfc_drive   = GPIO_0_0_tri_io[5];
assign  pfc_ok      = GPIO_0_0_tri_io[6];
assign  pfc_rvd     = GPIO_0_0_tri_io[7];



clk_wiz_0  clk_wiz_0_inst
(
   // Clock out ports
   .clk_out1_50m               (pll_clk_50m           ),     // output clk_out1_50m
   .clk_out2_200m              (pll_clk_200m          ),     // output clk_out2_200m
   .clk_out3_10m               (pll_clk_10m           ),
   .clk_out4_25m               (pll_clk_25m           ),
   .clk_out5_100m              (pll_clk_100m          ),     // output clk_out5_100m
   // Status and control signals
   .resetn                     (pll_reset_n           ),     // input resetn
   .locked                     (pll_locked            ),     // output locked
   // Clock in ports
   .clk_in1_50m                (ps2pl_clk50m_0        )
);

reg [31:0]  test_cnt;
always@(posedge pll_clk_10m or negedge pl_reset_n) begin
    if (!pl_reset_n) begin
        test_cnt <= 32'd0;
    end else begin
        if (test_cnt < 32'd5000000 - 32'd1) begin
            test_cnt <= test_cnt + 1'b1;
        end else begin
            test_cnt <= 32'd0;
        end
    end
end


// block design instant
system_wrapper      system_wrapper_inst
(   
    .DDR_addr                 (DDR_addr                 ),
    .DDR_ba                   (DDR_ba                   ),
    .DDR_cas_n                (DDR_cas_n                ),
    .DDR_ck_n                 (DDR_ck_n                 ),
    .DDR_ck_p                 (DDR_ck_p                 ),
    .DDR_cke                  (DDR_cke                  ),
    .DDR_cs_n                 (DDR_cs_n                 ),
    .DDR_dm                   (DDR_dm                   ),
    .DDR_dq                   (DDR_dq                   ),
    .DDR_dqs_n                (DDR_dqs_n                ),
    .DDR_dqs_p                (DDR_dqs_p                ),
    .DDR_odt                  (DDR_odt                  ),
    .DDR_ras_n                (DDR_ras_n                ),
    .DDR_reset_n              (DDR_reset_n              ),
    .DDR_we_n                 (DDR_we_n                 ),
    .FIXED_IO_ddr_vrn         (FIXED_IO_ddr_vrn         ),
    .FIXED_IO_ddr_vrp         (FIXED_IO_ddr_vrp         ),
    .FIXED_IO_mio             (FIXED_IO_mio             ),
    .FIXED_IO_ps_clk          (FIXED_IO_ps_clk          ),
    .FIXED_IO_ps_porb         (FIXED_IO_ps_porb         ),
    .FIXED_IO_ps_srstb        (FIXED_IO_ps_srstb        ),
    .LIT_memrd_addr_o_0       (LIT_memrd_addr           ),
    .LIT_memrd_en_o_0         (LIT_memrd_en             ),
    .LIT_memwr_addr_o_0       (LIT_memwr_addr           ),
    .LIT_memwr_dat_o_0        (LIT_memwr_dat            ),
    .LIT_memwr_en_o_0         (LIT_memwr_en             ),
    .LIT_memwr_strb_o_0       (LIT_memwr_strb           ),
    .REG_bresp_slverr_i_0     (REG_bresp_slverr         ),
    .REG_itf_rd_done_i_0      (REG_itf_rd_done          ),
    .REG_itf_rdata_i_0        (REG_itf_rdata            ),
    .REG_itf_wr_done_i_0      (REG_itf_wr_done          ),
    .REG_rresp_slverr_i_0     (REG_rresp_slverr         ),
    .GPIO_0_0_tri_io          (GPIO_0_0_tri_io          ),
    .ps2pl_clk100m_0          (ps2pl_clk100m_0          ),
    .ps2pl_clk50m_0           (ps2pl_clk50m_0           ),
    .ps2pl_resetn_0           (ps2pl_resetn_0           ),
    //Slave Interface Read Address Ports
    .pl2ps_axi_0_araddr       (s_axi_araddr             ),
    .pl2ps_axi_0_arburst      (s_axi_arburst            ),
    .pl2ps_axi_0_arcache      (s_axi_arcache            ),
    .pl2ps_axi_0_arlen        (s_axi_arlen              ),
    .pl2ps_axi_0_arlock       (s_axi_arlock             ),
    .pl2ps_axi_0_arprot       (s_axi_arprot             ),
    .pl2ps_axi_0_arqos        (s_axi_arqos              ),
//    .pl2ps_axi_0_arregion(s_axi_arregion      ),
    .pl2ps_axi_0_arready      (s_axi_arready            ),
    .pl2ps_axi_0_arsize       (s_axi_arsize             ),
    .pl2ps_axi_0_arvalid      (s_axi_arvalid            ),
    //Slave Interface Write Address Ports
    .pl2ps_axi_0_awaddr       (s_axi_awaddr             ),
    .pl2ps_axi_0_awburst      (s_axi_awburst            ),
    .pl2ps_axi_0_awcache      (s_axi_awcache            ),
    .pl2ps_axi_0_awlen        (s_axi_awlen              ),
    .pl2ps_axi_0_awlock       (s_axi_awlock             ),
    .pl2ps_axi_0_awprot       (s_axi_awprot             ),
    .pl2ps_axi_0_awqos        (s_axi_awqos              ),
//    .pl2ps_axi_0_awregion(s_axi_awregion      ),
    .pl2ps_axi_0_awready      (s_axi_awready            ),
    .pl2ps_axi_0_awsize       (s_axi_awsize             ),
    .pl2ps_axi_0_awvalid      (s_axi_awvalid            ),
    //Slave Interface Write Response Ports
    .pl2ps_axi_0_bready       (s_axi_bready             ),
    .pl2ps_axi_0_bresp        (s_axi_bresp              ),
    .pl2ps_axi_0_bvalid       (s_axi_bvalid             ),
    //Slave Interface Read Data Ports
    .pl2ps_axi_0_rdata        (s_axi_rdata              ),
    .pl2ps_axi_0_rlast        (s_axi_rlast              ),
    .pl2ps_axi_0_rready       (s_axi_rready             ),
    .pl2ps_axi_0_rresp        (s_axi_rresp              ),
    .pl2ps_axi_0_rvalid       (s_axi_rvalid             ),
    //Slave Interface Write Data Ports
    .pl2ps_axi_0_wdata        (s_axi_wdata              ),
    .pl2ps_axi_0_wlast        (s_axi_wlast              ),
    .pl2ps_axi_0_wready       (s_axi_wready             ),
    .pl2ps_axi_0_wstrb        (s_axi_wstrb              ),
    .pl2ps_axi_0_wvalid       (s_axi_wvalid             ),
    //Slave Interface ACLK RESET
    .pl2ps_axi_aclk_0         (s_axi_aclk               ),
    .pl2ps_axi_resetn_0       (s_axi_resetn             )
);


register #(
    .CSR_ADDR_SIZE(CSR_ADDR_SIZE)
) U_register(
    .RST_rstn_i               (pl_reset_n              ),
    .CLK_SYS_I                (ps2pl_clk50m_0          ),
    .rx_data_clk              (pll_clk_200m            ),
    .LIT_memwr_en_i           (LIT_memwr_en            ),
    .LIT_memwr_addr_i         (LIT_memwr_addr          ),
    .LIT_memwr_dat_i          (LIT_memwr_dat           ),
    .LIT_memwr_strb_i         (LIT_memwr_strb          ),
    .LIT_memrd_en_i           (LIT_memrd_en            ),
    .LIT_memrd_addr_i         (LIT_memrd_addr          ),
    .REG_itf_wr_done_o        (REG_itf_wr_done         ),
    .REG_bresp_slverr_o       (REG_bresp_slverr        ),
    .REG_itf_rd_done_o        (REG_itf_rd_done         ),
    .REG_rresp_slverr_o       (REG_rresp_slverr        ),
    .REG_itf_rdata_o          (REG_itf_rdata           ),
    //data
    // pl to ps
    .cnt_500ms                (test_cnt                ),
    .vol_avg                  (slowrx_vol_avg          ),
    .cur_avg                  (slowrx_cur_avg          ),
    .vol_samp                 (slowrx_vol              ),
    .cur_samp                 (slowrx_cur              ),
    .pl_state                 (slowrx_state            ),
    .lowrx_err_cnt            (lowrx_err_cnt           ),
    .pl_data                  (pl_to_ps_data           ),
    .pl_data2                 (pl_to_ps_data2          ),
    .test_ddr_cnt             (pl_to_ps_test_ddr3      ),
    .test_ddr_awaddr          (s_axi_awaddr            ),

    // ps to pl
    .set_cmd                  (slowtx_cmd              ),
    .set_cmd2                 (slowtx_cmd2             ),
    .set_dac                  (slowtx_dacset           ),
    .set_samppara             (slowtx_samppara         ),
    .ps_cmd                   (ps_to_pl_cmd            ),
    .ps_cmd2                  (ps_to_pl_cmd2           ),
    .test_ddr3                (ps_to_pl_test_ddr3      ),
    .wave_para                (ps_to_pl_wave_para      )
);



// 向另一个 fpga 发送数据的慢速模块
// 间隔 5 个 tx_clk 发送一次
always@(posedge pll_clk_200m or negedge pl_reset_n) begin
    if (!pl_reset_n) begin
        slow_tx_interval_cnt <= 16'd0;
    end else begin
        if (slow_tx_busy == 1'b0) begin
            slow_tx_interval_cnt <= slow_tx_interval_cnt + 1'b1;
        end else begin
            slow_tx_interval_cnt <= 16'd0;
        end
    end
end
always@(posedge pll_clk_200m or negedge pl_reset_n) begin
    if (!pl_reset_n) begin
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
    .sys_clk             (pll_clk_200m      ),  // 200M
    .sys_rst_n           (pl_reset_n        ),
    .start               (slow_tx_start     ),
    .busy                (slow_tx_busy      ),
    .send_data1          (slowtx_cmd        ),
    .send_data2          (slowtx_dacset     ),
    .send_data3          (slowtx_samppara   ),
    .send_data4          (slowtx_cmd2       ),
    .send_data5          (5                 ),
    .send_data6          (6                 ),
    .send_data7          (7                 ),
    .send_data8          (8                 ),
    .send_data9          (9                 ),
    .send_data10         (10                ),
    .tx_clk              (slow_tx_clk       ),
    .tx_data             (slow_tx_data      )
);



// 接收间隔 3 个 tx_clk
com_between_fpga_slow_rx #(
    .OVERITME_CNT        (COM_FPGA_SLOW_TXCLK_DIV_CNT * (5 - 2)),
    .DATA_NUM            (COM_FPGA_SLOW_DATA_NUM)      
) slow_rx_inst
(   
    .sys_clk             (pll_clk_200m      ),  // 200M
    .sys_rst_n           (pl_reset_n        ),
    .rx_clk              (slow_rx_clk       ),  // 对端发来的 clk
    .rx_data             (slow_rx_data      ),  // 对端发来的 data
    .valid               (),  // 数据接收成功标志
    .err_cnt             (lowrx_err_cnt     ),
    .rx_data1            (slowrx_vol_avg    ),
    .rx_data2            (slowrx_cur_avg    ),
    .rx_data3            (slowrx_vol        ),
    .rx_data4            (slowrx_cur        ),
    .rx_data5            (slowrx_state      ),
    .rx_data6            (),
    .rx_data7            (),
    .rx_data8            (),
    .rx_data9            (),
    .rx_data10           () 
);






assign  pl_to_ps_test_ddr3[15:0] = test_wr_fifo_cnt;
assign  pl_to_ps_test_ddr3[31:16] = test_rd_fifo_cnt;


fifo_axi4_adapter #(
  .FIFO_DW                (16                   ),
  .WR_AXI_BYTE_ADDR_BEGIN (DDR_BASE_ADDR  ),
  .WR_AXI_BYTE_ADDR_END   (DDR_BASE_ADDR + DDR_LENTH - 1),
  .RD_AXI_BYTE_ADDR_BEGIN (DDR_BASE_ADDR ),
  .RD_AXI_BYTE_ADDR_END   (DDR_BASE_ADDR + DDR_LENTH - 1),
  .AXI_DATA_WIDTH         (64                   ),
  .AXI_ADDR_WIDTH         (32                   ),
  .AXI_ID                 (4'b0000              ),
  .AXI_BURST_LEN          (8'd15                ),  //axi burst length = 16
  .FIFO_ADDR_DEPTH        (64                   )
)fifo_axi4_adapter_inst
(
  //clock reset
  .clk                 (pll_clk_100m        ),
  .reset               (~pl_reset_n         ),
  //wr_fifo Interface
  .wrfifo_clr          (wrfifo_clr          ),
  .wrfifo_clk          (wrfifo_clk          ),
  .wrfifo_wren         (wrfifo_wren         ),
  .wrfifo_din          (wrfifo_din          ),
  .wrfifo_full         (                    ),
  .wrfifo_wr_cnt       (test_wr_fifo_cnt    ),
  //rd_fifo Interface
  .rdfifo_clr          (wrfifo_clr          ),
  .rdfifo_clk          (wrfifo_clk          ),
  .rdfifo_rden         (rdfifo_rden         ),
  .rdfifo_dout         (rdfifo_dout         ),
  .rdfifo_empty        (                    ),
  .rdfifo_rd_cnt       (test_rd_fifo_cnt    ),
  // Master Interface Write Address Ports
  .m_axi_awid          (s_axi_awid          ),
  .m_axi_awaddr        (s_axi_awaddr        ),
  .m_axi_awlen         (s_axi_awlen         ),
  .m_axi_awsize        (s_axi_awsize        ),
  .m_axi_awburst       (s_axi_awburst       ),
  .m_axi_awlock        (s_axi_awlock        ),
  .m_axi_awcache       (s_axi_awcache       ),
  .m_axi_awprot        (s_axi_awprot        ),
  .m_axi_awqos         (s_axi_awqos         ),
  .m_axi_awregion      (s_axi_awregion      ),
  .m_axi_awvalid       (s_axi_awvalid       ),
  .m_axi_awready       (s_axi_awready       ),
  // Master Interface Write Data Ports
  .m_axi_wdata         (s_axi_wdata         ),
  .m_axi_wstrb         (s_axi_wstrb         ),
  .m_axi_wlast         (s_axi_wlast         ),
  .m_axi_wvalid        (s_axi_wvalid        ),
  .m_axi_wready        (s_axi_wready        ),
  // Master Interface Write Response Ports
  .m_axi_bid           (4'b0000             ),
  .m_axi_bresp         (s_axi_bresp         ),
  .m_axi_bvalid        (s_axi_bvalid        ),
  .m_axi_bready        (s_axi_bready        ),
  // Master Interface Read Address Ports
  .m_axi_arid          (s_axi_arid          ),
  .m_axi_araddr        (s_axi_araddr        ),
  .m_axi_arlen         (s_axi_arlen         ),
  .m_axi_arsize        (s_axi_arsize        ),
  .m_axi_arburst       (s_axi_arburst       ),
  .m_axi_arlock        (s_axi_arlock        ),
  .m_axi_arcache       (s_axi_arcache       ),
  .m_axi_arprot        (s_axi_arprot        ),
  .m_axi_arqos         (s_axi_arqos         ),
  .m_axi_arregion      (s_axi_arregion      ),
  .m_axi_arvalid       (s_axi_arvalid       ),
  .m_axi_arready       (s_axi_arready       ),
  // Master Interface Read Data Ports
  .m_axi_rid           (4'b0000             ),
  .m_axi_rdata         (s_axi_rdata         ),
  .m_axi_rresp         (s_axi_rresp         ),
  .m_axi_rlast         (s_axi_rlast         ),
  .m_axi_rvalid        (s_axi_rvalid        ),
  .m_axi_rready        (s_axi_rready        )
);




com_between_fpga_fast_rx  samp_rx_inst1(   
    .sys_clk             (pll_clk_200m       ),  // 200M
    .sys_rst_n           (pl_reset_n         ),
    .rx_clk              (fast_rx_clk        ),
    .rx_cs_n             (fast_rx_cs_n       ),
    .rx_dataa            (fast_rx_dataa      ),
    .rx_datab            (fast_rx_datab      ),
    .rx_vol_data         (fast_rx_vol_data   ),   // 接收到的电压数据
    .rx_cur_data         (fast_rx_cur_data   ),   // 接收到的电流数据
    .rx_done             (fast_rx_done       )    // 一帧接收完成标志
);


wire                 wr_ddr3_update         ;
wire  [15:0]         wr_ddr3_data           ;
wire                 wr_ddr3_en             ;
wire  [ 1:0]         wr_ddr3_range_sw       ;

assign      wr_ddr3_update     = ps_to_pl_test_ddr3[0];
assign      wr_ddr3_data       = ps_to_pl_test_ddr3[16:1];
assign      wr_ddr3_en         = ps_to_pl_test_ddr3[17];
assign      wr_ddr3_range_sw   = ps_to_pl_test_ddr3[19:18];

wr_ddr3_ctrl  wr_ddr3_ctrl_inst123
(   
    .sys_clk            (pll_clk_100m       ),  // xxM
    .sys_rst_n          (pl_reset_n         ),
    .samp_update        (fast_rx_done       ),
    .samp_cur           (fast_rx_cur_data   ),
    .samp_vol           (fast_rx_vol_data   ),
    .en                 (wr_ddr3_en         ),
    .range_sw           (wr_ddr3_range_sw   ),
    .div                (wave_div           ),
    .total_num          (wave_num           ),
    .ok                 (wave_ok            ),
    .wrfifo_clr         (wrfifo_clr         ),
    .wrfifo_clk         (wrfifo_clk         ),
    .wrfifo_din         (wrfifo_din         ),
    .wrfifo_wren        (wrfifo_wren        )
);




endmodule
