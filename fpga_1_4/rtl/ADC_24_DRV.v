//************************************************//
//FILE NAME:        ADC_24_DRV.V
//DESCRIPTION:      ADC_24_DRV模块
//AUTHOR:           hehang
//DATE:             04/18/24
//LOG:              ADC采样模块
//************************************************//

//************** Module Define *******************//
module ADC_24_DRV
(   
    input                       sys_clk,           //时钟  200M
    input                       sys_rst,           //复位信号，低有效
    input                       en,                //模块使能信号，高有效
    
    input                       misoA,           //主机输入
    input                       misoB,           //主机输入
    output  reg[23:0]           dataA,           //读取到的24bitADC数据
    output  reg[23:0]           dataB,           //读取到的24bitADC数据
    output  reg                 cs_n,            //片选
    output  reg                 cnv,             //转换信号
    output  reg                 sck              //时钟
);

//*************** Parameter & Internal Register ****//
// Parameter
parameter  SAMPLE_FREQ = 16'd400; //0.5M 2us 
parameter  SAMPLE_FREQ_DIV2 = SAMPLE_FREQ/2; //边沿之间计数
parameter  WAIT_CSN = 4'd2; //CNV上升沿后等待10ns，再拉低cs_n   仿真为25ns
parameter  WAIT_SCK = 4'd8; //拉低cs_n后第一个sck   仿真为65
parameter  SCK_FREQ = 4'd3; //周期30ns  3*5*2 = 30


parameter  S_FIRST = 7'd0;
parameter  S_IDEL = 7'd1;
parameter  S_WAITCSN = 7'd2;
parameter  S_WAITSLK = 7'd3;
parameter  S_READ = 7'd4;

// Reg
reg[3:0] cnt_waitsck;
reg[3:0] cnt_waitcsn;
reg[31:0] cnt_waitreset;
reg[7:0] bit_cnt = 8'd0;
reg[15:0] cnt_cnv;
reg cnv_posflag;
reg[3:0] cnv_num;
//*************** Main Code ***********************//   
//状态机
reg[7:0] state;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        state <= S_FIRST;
    end
    else begin
        if(en == 1'b1) begin
            case(state)
                S_FIRST: begin  //开始前先发几次CNV信号
                    if(cnv_num == 4'd10) begin
                        state <= S_IDEL;
                    end
                    else begin
                        state <= S_FIRST;
                    end
                end
                S_IDEL: begin   //等待CNV上升沿
                    if(cnv_posflag == 1'b1) begin
                        state <= S_WAITCSN;
                    end
                    else begin
                        state <= S_IDEL;
                    end
                end
                S_WAITCSN: begin //CNV上升沿延时10ns后拉低csn
                    if(cnt_waitcsn == WAIT_CSN) begin
                        state <= S_WAITSLK;
                    end
                    else begin
                        state <= S_WAITCSN;
                    end
                end
                S_WAITSLK: begin //拉低csn后延时20ns，开始输出sck
                    if(cnt_waitsck == WAIT_SCK) begin
                        state <= S_READ;
                    end
                    else begin
                        state <= S_WAITSLK;
                    end
                end
                S_READ: begin //开始读取数据
                    if(bit_cnt == 8'd24) begin
                        state <= S_IDEL;
                    end
                    else begin
                        state <= S_READ;
                    end
                end
                default: state <= S_FIRST;
            endcase

        end
        else begin
            // initial regs when the module is disabled 
            state <= S_FIRST;
        end
    end
end






//产生CNV信号，并且在上升沿时产生一个标志位
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_cnv <= 16'd0;
        cnv <= 1'b0;
        cnv_posflag <= 1'b0;
        cnv_num <= 4'd0;
    end
    else begin
        if(en == 1'b1) begin
            if(cnt_cnv == SAMPLE_FREQ_DIV2 - 16'd1) begin
                cnv <= 1'b1;
                cnt_cnv <= cnt_cnv + 16'd1;
                cnv_posflag <= 1'b1;
                if(cnv_num == 4'd10) begin
                    cnv_num <= cnv_num;
                end
                else begin
                    cnv_num <= cnv_num + 4'd1;
                end
            end
            else if(cnt_cnv == SAMPLE_FREQ - 16'd1) begin
                cnv <= 1'b0;
                cnt_cnv <= 16'd0;
                cnv_posflag <= 1'b0;
            end
            else begin
                cnv <= cnv;
                cnt_cnv <= cnt_cnv + 16'd1;
                cnv_posflag <= 1'b0;
            end
        end
        else begin
            // initial regs when disable module
            cnt_cnv <= 16'd0;
            cnv <= 1'b0;
            cnv_posflag <= 1'b0;
            cnv_num <= 4'd0;
        end
    end
end







//在cnv上升沿之后10ns 开始拉低cs_n，开始读取上一次转换结果
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_waitcsn <= 4'd0;
    end
    else begin
        if(state == S_WAITCSN) begin
            cnt_waitcsn <= cnt_waitcsn + 4'd1;
        end
        else begin
            cnt_waitcsn <= 4'd0;
        end
    end
end


//拉低cs_n后延时20ns再开始产生sck
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_waitsck <= 4'd0;
    end
    else begin
        if(state == S_WAITSLK) begin
            cnt_waitsck <= cnt_waitsck + 4'd1;
        end
        else begin
            cnt_waitsck <= 4'd0;
        end
    end
end


//产生sck信号, 周期40ns
reg[3:0] cnt_sck;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        sck <= 1'b0;
        cnt_sck <= 4'd0;
    end
    else begin
        if(state == S_READ) begin
            if(cnt_sck == SCK_FREQ - 4'd1) begin
                sck <= ~sck;
                cnt_sck <= 4'd0;
            end
            else begin
                cnt_sck <= cnt_sck + 4'd1;
            end
            
        end
        else begin
            sck <= 1'b0;
        end
    end
end


reg   sck_r1;
reg   sck_r2;
wire  sck_pedge;

always@(posedge sys_clk or negedge sys_rst) begin
    if (!sys_rst) begin
        sck_r1 <= 1'b0;
        sck_r2 <= 1'b0;
    end else begin
        sck_r1 <= sck;
        sck_r2 <= sck_r1;
    end
end

assign   sck_pedge = sck_r1 & (~sck_r2);

reg   cs_n_r1;
reg   cs_n_r2;
wire  cs_n_pedge;

always@(posedge sys_clk or negedge sys_rst) begin
    if (!sys_rst) begin
        cs_n_r1 <= 1'b1;
        cs_n_r2 <= 1'b1;
    end else begin
        cs_n_r1 <= cs_n;
        cs_n_r2 <= cs_n_r1;
    end
end

assign   cs_n_pedge = cs_n_r1 & (~cs_n_r2);

//在sck上升沿读取数据
reg[23:0] dataA_r = 24'd0, dataB_r = 24'd0;
always@(posedge sys_clk or negedge sys_rst) begin
    if (!sys_rst) begin
        bit_cnt <= 8'd0;
        dataA_r <= 24'd0;
        dataB_r <= 24'd0;
    end else begin
        if (cs_n_pedge == 1'b1) begin
            bit_cnt <= 8'd0;
        end else if(sck_pedge == 1'b1) begin
            dataA_r <= {dataA_r[22:0], misoA};
            dataB_r <= {dataB_r[22:0], misoB};
            bit_cnt <= bit_cnt + 8'd1;
        end
    end
end



// //在sck上升沿读取数据
// reg[23:0] dataA_r = 24'd0, dataB_r = 24'd0;
// always@(posedge sck or posedge cs_n) begin
//     if(cs_n == 1'b1) begin
//     //initial regs
//         bit_cnt <= 8'd0;
//     end
//     else begin
//         dataA_r <= {dataA_r[22:0], misoA};
//         dataB_r <= {dataB_r[22:0], misoB};
//         bit_cnt <= bit_cnt + 8'd1;
//     end
// end


//在每次读取完后改变输出data
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        dataA <= 24'd0;
        dataB <= 24'd0;
    end
    else begin
        if(bit_cnt == 8'd24) begin
            dataA <= dataA_r;
            dataB <= dataB_r;
        end
        else begin
            dataA <= dataA;
            dataB <= dataB;
        end
    end
end



//cs_n信号
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cs_n <= 1'b1;
    end
    else begin
        if((state == S_READ) || (state == S_WAITSLK)) begin
            cs_n <= 1'b0;
        end
        else begin
            cs_n <= 1'b1;
        end
    end
end









    
endmodule       

//*************** END OF LINE ********************//