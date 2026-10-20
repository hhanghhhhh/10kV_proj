module register #(
    parameter CSR_ADDR_SIZE = 12
)(
    input                       RST_rstn_i             ,
    input                       CLK_SYS_I              ,
    input                       rx_data_clk            ,
	
    input                       LIT_memwr_en_i         ,
    input  [CSR_ADDR_SIZE-1:0]  LIT_memwr_addr_i       ,
    input  [31:0]               LIT_memwr_dat_i        ,
    input  [3:0]                LIT_memwr_strb_i       ,
    input                       LIT_memrd_en_i         ,
    input  [CSR_ADDR_SIZE-1:0]  LIT_memrd_addr_i       ,
    
    output                      REG_itf_wr_done_o      ,
    output                      REG_bresp_slverr_o     ,
    output                      REG_itf_rd_done_o      ,
    output                      REG_rresp_slverr_o     ,
    output reg [31:0]           REG_itf_rdata_o        ,
    
    // pl to ps
    input      [31:0]           cnt_500ms              ,
    input      [31:0]           vol_avg                ,
    input      [31:0]           cur_avg                ,
    input      [31:0]           vol_samp               ,
    input      [31:0]           cur_samp               ,
    input      [31:0]           pl_state               ,
    input      [31:0]           lowrx_err_cnt          ,
    input      [31:0]           pl_data                ,
    input      [31:0]           pl_data2               ,
    input      [31:0]           test_ddr_cnt           ,
    input      [31:0]           test_ddr_awaddr        ,
    
    // ps to pl
    output reg [31:0]           set_cmd                ,
    output reg [31:0]           set_dac                ,
    output reg [31:0]           set_samppara           ,
    output reg [31:0]           set_cmd2               ,
    output reg [31:0]           set_para               ,
    output reg [31:0]           set_para2              ,
    output reg [31:0]           ps_cmd                 ,
    output reg [31:0]           ps_cmd2                ,
    output reg [31:0]           test_ddr3              ,
    output reg [31:0]           wave_para              
);

reg [31:0]           cnt_500ms_r              ;
reg [31:0]           pl_data_r                ;
reg [31:0]           pl_data2_r               ;
reg [31:0]           test_ddr_cnt_r           ;
reg [31:0]           test_ddr_awaddr_r        ;

wire [31:0]           vol_avg_r                ;
wire [31:0]           cur_avg_r                ;
wire [31:0]           vol_samp_r               ;
wire [31:0]           cur_samp_r               ;
wire [31:0]           pl_state_r               ;
wire [31:0]           lowrx_err_cnt_r          ;


always@(posedge CLK_SYS_I) begin
    cnt_500ms_r              <=     cnt_500ms              ;
    pl_data_r                <=     pl_data                ;
    pl_data2_r               <=     pl_data2               ;
    test_ddr_cnt_r           <=     test_ddr_cnt           ;
    test_ddr_awaddr_r        <=     test_ddr_awaddr        ;
end


// AXI LITE 接口读写响应结果，0 代表读写成功
assign  REG_bresp_slverr_o = 1'b0;
assign  REG_rresp_slverr_o = 1'b0;


// ps to pl 
always@(posedge CLK_SYS_I or negedge RST_rstn_i) begin
    if (!RST_rstn_i) begin
        set_cmd         <= 32'd0;
        set_dac         <= 32'd0;
        set_samppara    <= 32'd0;
        set_cmd2        <= 32'd0;
        set_para        <= 32'd0;
        set_para2       <= 32'd0;
        ps_cmd          <= 32'd0;
        ps_cmd2         <= 32'd0;
        test_ddr3       <= 32'd0;
        wave_para       <= 32'd0;
    end else begin
        if (LIT_memwr_en_i) begin
            case (LIT_memwr_addr_i[CSR_ADDR_SIZE-1:2])
                0: set_cmd      <= LIT_memwr_dat_i;
                1: set_dac      <= LIT_memwr_dat_i;
                2: set_samppara <= LIT_memwr_dat_i;
                3: set_cmd2     <= LIT_memwr_dat_i;
                4: set_para     <= LIT_memwr_dat_i;
                5: set_para2    <= LIT_memwr_dat_i;
                6: ps_cmd       <= LIT_memwr_dat_i;
                7: ps_cmd2      <= LIT_memwr_dat_i;
                8: test_ddr3    <= LIT_memwr_dat_i;
                9: wave_para    <= LIT_memwr_dat_i;
            endcase
        end
    end
end

// pl to ps 
always@(posedge CLK_SYS_I or negedge RST_rstn_i) begin
    if (!RST_rstn_i) begin
        REG_itf_rdata_o <= 32'd0;
    end else begin
        if (LIT_memrd_en_i) begin
            case (LIT_memrd_addr_i[CSR_ADDR_SIZE-1:2])
                0: REG_itf_rdata_o <= cnt_500ms_r         ;
                1: REG_itf_rdata_o <= vol_avg_r           ;
                2: REG_itf_rdata_o <= cur_avg_r           ;
                3: REG_itf_rdata_o <= vol_samp_r          ;
                4: REG_itf_rdata_o <= cur_samp_r          ;
                5: REG_itf_rdata_o <= pl_state_r          ;
                8: REG_itf_rdata_o <= lowrx_err_cnt_r     ;
                10:REG_itf_rdata_o <= pl_data_r           ;
                11:REG_itf_rdata_o <= pl_data2_r          ;
                12:REG_itf_rdata_o <= test_ddr_cnt_r      ;
                13:REG_itf_rdata_o <= test_ddr_awaddr_r   ;
                default: REG_itf_rdata_o <= 112233;    // invalid addr
            endcase
        end
    end
end



DFF_R #(.DWIDTH(1)) U_reg_wr_done (.RSTN(RST_rstn_i), .CLK(CLK_SYS_I), .D(LIT_memwr_en_i), .Q(REG_itf_wr_done_o));
DFF_R #(.DWIDTH(1)) U_reg_rd_done (.RSTN(RST_rstn_i), .CLK(CLK_SYS_I), .D(LIT_memrd_en_i), .Q(REG_itf_rd_done_o));




async_fifo  #(
	.DATA_WIDTH                     (32          ),	//FIFO位宽
	.DATA_DEPTH                     (4           )	//FIFO深度
)vol_avg_fifo_inst(				
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (vol_avg     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (vol_avg_r   )  //输出的数据
);
async_fifo  #(
	.DATA_WIDTH                     (32          ),			//FIFO位宽
	.DATA_DEPTH                     (4           )			//FIFO深度
)cur_avg_fifo_inst(		
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (cur_avg     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (cur_avg_r   )  //输出的数据
);
async_fifo  #(
	.DATA_WIDTH                     (32          ),			//FIFO位宽
	.DATA_DEPTH                     (4           )			//FIFO深度
)vol_samp_fifo_inst(				
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (vol_samp     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (vol_samp_r   )  //输出的数据
);
async_fifo  #(
	.DATA_WIDTH                     (32          ),			//FIFO位宽
	.DATA_DEPTH                     (4           )			//FIFO深度
)cur_samp_fifo_inst(				
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (cur_samp     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (cur_samp_r   )  //输出的数据
);
async_fifo  #(
	.DATA_WIDTH                     (32          ),			//FIFO位宽
	.DATA_DEPTH                     (4           )			//FIFO深度
)pl_state_fifo_inst(				
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (pl_state     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (pl_state_r   )  //输出的数据
);
async_fifo  #(
	.DATA_WIDTH                     (32          ),			//FIFO位宽
	.DATA_DEPTH                     (4           )			//FIFO深度
)lowrx_err_cnt_fifo_inst(				
    .rst_n                          (RST_rstn_i  ),
    .wr_clk		                    (rx_data_clk ), //写时钟
    .data_in	                    (lowrx_err_cnt     ), //写入的数据
    .rd_clk		                    (CLK_SYS_I   ), //读时钟					                                        
    .data_out	                    (lowrx_err_cnt_r   )  //输出的数据
);








endmodule
