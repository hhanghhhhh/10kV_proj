/************************************************
//FILE NAME:        com_between_fpga_fast_tx.v
//DESCRIPTION:      com_between_fpga_fast_tx
//AUTHOR:           hehang
//DATE:             25/06/20
//LOG:              目前 CLK_DIV = 8 时，速度刚好是 1M 整，略小几十 ns
************************************************/
//************** Module Define *******************//
module com_between_fpga_fast_tx #(
    parameter CLK_DIV = 10
)
(   
    input                       sys_clk             ,  // 200M
    input                       sys_rst_n           ,
    input                       cnv_flag            ,
    input           [23:0]      vol_samp            ,
    input           [23:0]      cur_samp            ,

    output  wire                tx_clk              ,
    output  reg                 tx_cs_n             ,
    output  reg                 tx_dataa            ,
    output  reg                 tx_datab            
);

//*************** Parameter & Internal Register ****//
// parameter
localparam      S_IDLE    = 0,
                S_READ    = 1,
                S_LOAD    = 2,
                S_SEND    = 3,
                S_DONE    = 4,
                S_WAIT_CS = 5;


// reg
wire            cnv_flag_pedge  ;
wire            fifo_clkwr      ;
wire            fifo_clkrd      ;
reg             fifo_wren       ;
reg             fifo_rden       ;
reg     [23:0]  fifo_vol_wrdata ;
wire    [23:0]  fifo_vol_rddata ;
reg     [23:0]  fifo_cur_wrdata ;
wire    [23:0]  fifo_cur_rddata ;
wire    [ 7:0]  fifo_wr_cnt     ;
wire    [ 7:0]  fifo_rd_cnt     ;

reg     [ 3:0]  state           ;
reg     [23:0]  shift_vol       ; 
reg     [23:0]  shift_cur       ;
reg     [ 4:0]  bit_cnt         ;
reg     [15:0]  tx_clk_div_cnt  ;
reg             tx_clk_pedge    ;
reg             tx_clk_nedge    ;
reg             tx_clk_tmp      ;


//*************** Main Code ***********************//
assign      fifo_clkwr = sys_clk;
assign      fifo_clkrd = sys_clk;
assign      tx_clk = (state == S_SEND) ? tx_clk_tmp : 1'b0;

// 将每次采样数据写入 fifo 中
always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        fifo_wren <= 1'b0;
        fifo_vol_wrdata <= 24'd0;
        fifo_cur_wrdata <= 24'd0;
    end else begin
        if (cnv_flag_pedge == 1'b1) begin
            fifo_wren <= 1'b1;
            fifo_vol_wrdata <= vol_samp;
            fifo_cur_wrdata <= cur_samp;
        end
        else begin
            fifo_wren <= 1'b0;
        end
    end
end

// tx_clk
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        tx_clk_div_cnt <= (CLK_DIV >> 1) - 1'b1;        //  缩短发送时间，下降沿接收，不影响
        tx_clk_tmp <= 1'b0;
        tx_clk_pedge <= 1'b0;
        tx_clk_nedge <= 1'b0;
    end else if (state == S_SEND || state == S_WAIT_CS) begin
        if (tx_clk_div_cnt == ((CLK_DIV >> 1) - 1'b1)) begin
            tx_clk_div_cnt <= tx_clk_div_cnt + 1'b1;
            tx_clk_tmp <= 1'b1;
            tx_clk_pedge <= 1'b1;
            tx_clk_nedge <= 1'b0;
        end else if(tx_clk_div_cnt == (CLK_DIV - 1'b1)) begin 
            tx_clk_div_cnt <= 1'b0;
            tx_clk_tmp <= 1'b0;
            tx_clk_pedge <= 1'b0;
            tx_clk_nedge <= 1'b1;
        end else begin
            tx_clk_div_cnt <= tx_clk_div_cnt + 1'b1;
            tx_clk_pedge <= 1'b0;
            tx_clk_nedge <= 1'b0;
        end
    end else begin
        tx_clk_div_cnt <= (CLK_DIV >> 1) - 1'b1;
        tx_clk_tmp <= 1'b0;
        tx_clk_pedge <= 1'b0;
        tx_clk_nedge <= 1'b0;
    end
end




always@(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        state <= S_IDLE;
        tx_cs_n   <= 1'b1;
        fifo_rden <= 1'b0;
        shift_vol <= 24'd0;
        shift_cur <= 24'd0;
        tx_dataa  <= 1'b0;
        tx_datab  <= 1'b0;
        bit_cnt   <= 5'd0;
    end else begin
        case (state)
            S_IDLE: begin
                tx_cs_n <= 1'b1;
                if (fifo_rd_cnt != 1'b0) begin
                    fifo_rden <= 1'b1;
                    state <= S_READ;
                end
            end
            S_READ: begin
                fifo_rden <= 1'b0;  // 只拉高一个clk
                state <= S_LOAD;
            end
            S_LOAD: begin
                shift_vol <= fifo_vol_rddata;
                shift_cur <= fifo_cur_rddata;
                bit_cnt <= 5'd24;
                tx_cs_n <= 1'b0;
                state <= S_SEND;
            end
            S_SEND: begin
                // 在 tx_clk 上升沿移位
                if (tx_clk_pedge) begin
                    tx_dataa <= shift_vol[23];
                    tx_datab <= shift_cur[23];
                    shift_vol <= {shift_vol[22:0],1'b0};
                    shift_cur <= {shift_cur[22:0],1'b0};
                    bit_cnt <= bit_cnt - 1'b1;
                end
                // 最后一个下降沿发完，进入等待
                if (tx_clk_nedge && (bit_cnt == 0)) begin
                    state <= S_WAIT_CS;
                end
            end
            S_WAIT_CS: begin
                if (tx_clk_pedge) begin
                    tx_cs_n <= 1;   // 延迟到下一个上升沿才释放
                    state   <= S_IDLE;
                end
            end
            default: state <= S_IDLE;
        endcase
    end
end



soft_fifo_samp  vol_samp_fifo(
    .rst            (~sys_rst_n     ),  //asynchronous port,active hight
    .clkw           (fifo_clkwr     ),  //write clock
    .clkr           (fifo_clkrd     ),  //read clock
    .we             (fifo_wren      ),  //write enable,active hight
    .di             (fifo_vol_wrdata),  //write data
    .re             (fifo_rden      ),  //read enable,active hight
    .dout           (fifo_vol_rddata),  //read data
    .valid          (       ),  //read data valid flag
    .full_flag      (       ),  //fifo full flag
    .empty_flag     (       ),  //fifo empty flag
    .afull          (       ),  //fifo almost full flag
    .aempty         (       ),  //fifo almost empty flag
    .wrusedw        (fifo_wr_cnt    ),  //stored data number in fifo
    .rdusedw        (fifo_rd_cnt    )   //available data number for read      
);


soft_fifo_samp  cur_samp_fifo(
    .rst            (~sys_rst_n     ),  //asynchronous port,active hight
    .clkw           (fifo_clkwr     ),  //write clock
    .clkr           (fifo_clkrd     ),  //read clock
    .we             (fifo_wren      ),  //write enable,active hight
    .di             (fifo_cur_wrdata),  //write data
    .re             (fifo_rden      ),  //read enable,active hight
    .dout           (fifo_cur_rddata),  //read data
    .valid          (       ),  //read data valid flag
    .full_flag      (       ),  //fifo full flag
    .empty_flag     (       ),  //fifo empty flag
    .afull          (       ),  //fifo almost full flag
    .aempty         (       ),  //fifo almost empty flag
    .wrusedw        (       ),  //stored data number in fifo
    .rdusedw        (       )   //available data number for read      
);




posedge_detect  cnv_flag_pedge_inst1
(
.sys_clk            (sys_clk            ),
.rst_n              (sys_rst_n          ),
.signal             (cnv_flag           ),
.signal_default     (1'b0               ),
.signal_pedge       (cnv_flag_pedge     )
);



endmodule
//*************** END OF LINE ********************//