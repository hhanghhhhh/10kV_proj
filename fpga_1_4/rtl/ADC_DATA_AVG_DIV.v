//************************************************//
//FILE NAME:        ADC_DATA_AVG.V
//DESCRIPTION:      ADC_DATA_AVG模块
//AUTHOR:           hehang
//DATE:             07/27/23
//LOG:              求平均值，输入数据为有符号数，若是无符号数，
//                  则需要将输入位宽扩展1位0，防止对大值被当作-1处理。
//************************************************//

//************** Module Define *******************//
module ADC_DATA_AVG_DIV
#
(   
    parameter   DATA_WIDTH = 24,
    parameter   ADD_WIDTH = 40
)
(   
    input                                           sys_clk,           //系统时钟  25M
    input                                           sys_rst,           //复位信号，低有效
    input                                           en,                //上升沿触发采样
    input[15:0]                                     sample_num,        //采样点数
    input[15:0]                                     interval_cnt,      //间隔多久取一个点,最少=50，2us， 最大65535
    input signed[DATA_WIDTH - 1 : 0]                adc_data,          //采样数据
    
    output  reg                                     data_ok,          //
    output  reg signed[DATA_WIDTH - 1 : 0]          data_avg_out,     //输出平均值
    output  reg signed[ADD_WIDTH - 1 : 0]           data_add_out     //输出累加和
);

//*************** Parameter & Internal Register ****//
// Parameter
localparam S_IDEL = 8'd0;
localparam S_SAMPLE = 8'd1;
localparam S_DIV = 8'd3;
localparam S_UPDATE = 8'd4;

// Reg
reg[15:0] cnt_sample_num;
reg[15:0] cnt_interval;
reg                             div_start;
wire                            div_done;
reg signed[ADD_WIDTH - 1:0]     data_add; //累加和
wire signed[ADD_WIDTH - 1 : 0]  tmp_avg;
reg[15:0] i_index;
reg sample_ok;
//*************** Main Code ***********************//   
//将ADC采样值打拍延时
reg signed[DATA_WIDTH - 1:0] adc_data_r[1:0];
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        adc_data_r[0] <= $signed('d0);
        adc_data_r[1] <= $signed('d0);
    end
    else begin
        adc_data_r[0] <= adc_data;
        adc_data_r[1] <= adc_data_r[0];
    end
end


//DSP输入的参数，在en以后不允许改变了
reg[15:0] sample_num_r, interval_cnt_r;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        sample_num_r <= 16'd0;
        interval_cnt_r <= 16'd0;
    end
    else begin
        if(en == 1'b1) begin
            sample_num_r <= sample_num_r;
            interval_cnt_r <= interval_cnt_r;
        end
        else begin
            // initial regs when the module is disabled 
            sample_num_r <= sample_num;
            interval_cnt_r <= interval_cnt;
        end
    end
end



//检测en信号上升沿，上升沿开始一次采样
reg en_r1, en_r2;
wire en_posedge;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        en_r1 <= 1'b0;
        en_r2 <= 1'b0;
    end
    else begin
        en_r1 <= en;
        en_r2 <= en_r1;
    end
end
assign en_posedge = en_r1 && (!en_r2);


//状态机
reg[7:0] state;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        state <= S_IDEL;
        data_add_out <= $signed('d0);
        div_start <= 1'b0;
        data_ok <= 1'b0;
    end
    else begin
        case(state)
            S_IDEL: begin     //等待en上升沿
                if(en_posedge == 1'b1) begin
                    data_ok <= 1'b0;
                    state <= S_SAMPLE;
                end
            end
            S_SAMPLE: begin   //采样完成后更新输出
                if(sample_ok == 1'b1) begin
                    data_add_out <= data_add;
                    div_start <= 1'b1;  //计算除法
                    state <= S_DIV;
                end
            end
            S_DIV: begin
                div_start <= 1'b0;
                if(div_done == 1'b1) begin
                    data_avg_out <= tmp_avg[DATA_WIDTH - 1 : 0];
                    data_ok <= 1'b1;
                    state <= S_IDEL;
                end
            end
            default: state <= S_IDEL;
        endcase
    end
end



//间隔多久取一个点
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_interval <= 16'd0;
    end
    else begin
        if(state == S_SAMPLE) begin
            if(cnt_interval == interval_cnt_r - 16'd1) begin
                cnt_interval <= 16'd0;
            end
            else begin
                cnt_interval <= cnt_interval + 16'd1;
            end
        end
        else begin
            // initial regs when disable module
            cnt_interval <= 16'd0;
        end
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        data_add <= $signed('d0);
        cnt_sample_num <= 16'd0;
        sample_ok <= 1'b0;
    end
    else begin
        if(state == S_SAMPLE) begin
            if(cnt_interval == 16'd1) begin
                if(cnt_sample_num < sample_num_r) begin
                    //计算累加和
                    data_add <= data_add + adc_data_r[1];
                    cnt_sample_num <= cnt_sample_num + 16'd1;
                end
                else begin
                    sample_ok <= 1'b1;
                end
            end
        end
        else if(state == S_IDEL) begin
            data_add <= $signed('d0);
        end
        else begin
            // initial regs when the module is disabled 
            cnt_sample_num <= 16'd0;
            sample_ok <= 1'b0;
        end
    end
end



divider_avg   divider_inst1
(
  .clk(sys_clk),
  .denominator(sample_num_r),   //分母   16bit  unsigned
  .numerator(data_add_out),   //分子  40bit  signed
  .rst(1'b0),
  .start(div_start),
  .done(div_done),
  .quotient(tmp_avg),    //商 40bit  signed
  .remainder()    //余数 16bit  unsigned
);




    
endmodule       

//*************** END OF LINE ********************//