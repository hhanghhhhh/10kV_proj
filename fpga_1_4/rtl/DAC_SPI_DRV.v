//************************************************//
//FILE NAME:        DAC_SPI_DRV.V
//DESCRIPTION:      DAC SPI DRV 模块
//AUTHOR:           hehang
//DATE:             20240417
//LOG:              sclk下降沿更新数据
//                  en为1期间仅执行一次,每次通讯大概4.5us以内（从en开始到ok为1）
//************************************************//

//************** Module Define *******************//
module DAC_SPI_DRV
#(
    parameter                   DW = 16        //数据位宽
)
(   
    input   wire                sys_clk,        //系统时钟 25M
    input   wire                sys_rst,        //复位信号，低有效
    input   wire                en,             //启动控制  由0置1开始一次通讯
    input   wire[DW - 1:0]      DATA,           //写入数据 16bit
    
    output  reg                 cs_n,           //片选信号，低有效
    output  reg                 sclk,           //SPI 时钟
    output  reg                 mosi,           //SPI 主机输出
    output  reg                 DAC_OK          //DACOK信号
);
//*************** Parameter & Internal Register ****//
// Parameter
localparam   FREQ_SCLK = 8;   //sclk频率 = sys_clk / FREQ_SCLK  6.25M
localparam   FREQ_SCLK_DIV2 = FREQ_SCLK / 2;
localparam   CNT_CS0_TO_SCLKFIRST = 10;     //cs0到第一个sclk的上升沿的时间
localparam   CNT_SCLKLAST_TO_CS1 = 10;  //最小2!   //最后一个sclk的下降沿到cs置1的时间   
localparam   CNT_CS1_TO_DACOK = 10;         //cs置1后隔多久dacok置1，代表可以打下一发 （即cs为1的最小时长）

// Reg
reg[7:0]        cnt_sclk;
reg[7:0]        cnt_bit;
reg[7:0]        cnt_edge_sclk;
reg[1:0]        state;
reg[7:0]        dly_sclk;   //CS_N置0，到，first rise sclk之间的延时计数
reg[7:0]        dly_cs;     //输出最后一个上升沿后，到，cs置1的延时计数
reg             sclk_flag;
//*************** Main Code ***********************//	
//锁存数据，当模块开始传输数据时，传过来的数不允许再变化了
reg[DW - 1:0] DATA_r;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        DATA_r <= 0;
    end
    else begin
        if(state == 1'b1) begin   //传输过程中锁存
            DATA_r <= DATA_r;
        end
        else begin
            DATA_r <= DATA;
        end
    end
end

//State Machine
always@(posedge sys_clk or negedge sys_rst)
begin
    if (sys_rst == 1'b0) begin
        state <= 2'd0;
    end
    else begin
        case(state)
            2'd0: begin    //使能信号触发一次传输
                if(en == 1'b1) begin
                    state <= 2'd1;
                end
                else begin
                    state <= 2'd0;
                end
            end 
            2'd1: begin    //传输过程
                if(cnt_edge_sclk == DW*2) begin
                    state <= 2'd2;
                end
                else begin
                    state <= 2'd1;
                end
            end
            2'd2: begin   //等待使能信号结束
                if(cs_n == 1'b1 && en == 1'b0) begin
                    state <= 2'd0;
                end
                else begin
                    state <= 2'd2;
                end
            end
            default: state <= 2'd0;
        endcase
    end
end
            
//CS 产生 以及DAC_OK
always@(posedge sys_clk or negedge sys_rst)
begin
    if (sys_rst == 1'b0) begin
        cs_n <= 1'b1;    
        dly_cs <= 8'b0;
        DAC_OK <= 1'b1;
    end
    else begin
        if(state == 2'd1) begin
            cs_n <= 1'b0;
            DAC_OK <= 1'b0;
        end    
        else if(state == 2'd2) begin     //在最后一个sclk下降沿延时一段时间后，再使cs_n置高
            if(dly_cs < CNT_SCLKLAST_TO_CS1 - 2) begin
                dly_cs <= dly_cs + 1'b1;
                cs_n <= 1'b0;
                DAC_OK <= 1'b0;
            end
            else if(dly_cs < CNT_SCLKLAST_TO_CS1 - 2 + CNT_CS1_TO_DACOK) begin
                dly_cs <= dly_cs + 1'b1;
                cs_n <= 1'b1;
                DAC_OK <= 1'b0;
            end
            else begin
                dly_cs <= dly_cs;
                cs_n <= 1'b1;
                DAC_OK <= 1'b1;
            end
        end
        else begin
            cs_n <= 1'b1;
            dly_cs <= 8'b0;
            DAC_OK <= 1'b1;
        end
    end 
end


//sclk 生成  ,在cs_n下降沿后，延时一段时间再产生第一个sclk上升沿，在最后一个sclk下降沿延时一段时间后，再使cs_n置高
always@(posedge sys_clk or negedge sys_rst)
begin
    if (sys_rst == 1'b0) begin
            cnt_sclk <= 8'd0;
            sclk <= 1'b0;
            cnt_edge_sclk <= 8'd0;
            dly_sclk <= 8'd0;
            sclk_flag <= 1'b0;
    end
    else begin
        if(state == 2'd1) begin
            //generate first negedge and delay before first rise
            if(dly_sclk < CNT_CS0_TO_SCLKFIRST) begin
                dly_sclk <= dly_sclk + 1'b1;
            end
            else begin
                // first rise
                if(sclk_flag == 1'b0) begin
                    sclk <= 1'b1;
                    sclk_flag <= 1'b1;
                    cnt_edge_sclk <= 8'd1;
                end
                else begin
                    if(cnt_sclk < FREQ_SCLK_DIV2 - 1'b1)
                        cnt_sclk <= cnt_sclk + 1'b1;
                    else begin
                        sclk <= ~sclk;
                        cnt_sclk <= 8'd0;
                        cnt_edge_sclk <= cnt_edge_sclk + 1'b1;
                    end
                end
            end
        end
        else begin
            cnt_sclk <= 8'b0;
            sclk <= 1'b0;
            cnt_edge_sclk <= 8'b0;
            dly_sclk <= 8'b0;
            sclk_flag <= 1'b0;
        end     
    end
end


//----------------------以下写法二选一----------------------//

//========sclk上升沿改变数据，平时是低电平=================//
//always@(posedge sclk  or posedge cs_n)
//begin
//    if(cs_n == 1'b1) begin
//        cnt_bit <= 8'd0;
//        mosi <= 1'b0;
//    end
//    else begin
//        cnt_bit <= cnt_bit + 1'b1;
//        mosi <= DATA_r[DW - cnt_bit - 1'b1];
//    end 
//end    
//==============end1=======================================//


//=======sclk下降沿改变数据，平时是低电平================//
//cs下降沿时产生一个周期的flag标志
reg  cs_n_flag;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cs_n_flag <= 1'b0;
    end
    else begin
        if((state == 2'd0) && (en == 1'b1)) begin
            cs_n_flag <= 1'b1;
        end
        else begin
            cs_n_flag <= 1'b0;
        end
    end
end


reg sclk_r1;
reg sclk_r2;
wire sclk_nedge;

always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        sclk_r1 <= 1'b0;
        sclk_r2 <= 1'b0;
    end
    else begin
        sclk_r1 <= sclk;
        sclk_r2 <= sclk_r1;
    end
end
assign sclk_nedge = (~sclk_r1) & sclk_r2;



//sclk下降沿更新数据 平时是低电平
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_bit <= 8'd1;
        mosi <= DATA_r[DW - 1'b1];
    end
    else begin
        if(cs_n_flag == 1'b1) begin
            cnt_bit <= 8'd1;
            mosi <= DATA_r[DW - 1'b1];
        end
        else if(sclk_nedge == 1'b1) begin
            if(cnt_bit == DW) begin //最后一个下降沿
                cnt_bit <= 1'b0;
                mosi <= 0;
            end
            else begin
                cnt_bit <= cnt_bit + 1'b1;
                mosi <= DATA_r[DW - cnt_bit - 1'b1];
            end
        end
    end
end



// //sclk下降沿更新数据 平时是低电平
// always@(negedge sclk  or posedge cs_n_flag)
// begin
//     if(cs_n_flag == 1'b1) begin
//         cnt_bit <= 8'd1;
//         mosi <= DATA_r[DW - 1'b1];
//     end
//     else begin
//         if(cnt_bit == DW) begin //最后一个下降沿
//             cnt_bit <= 1'b0;
//             mosi <= 0;
//         end
//         else begin
//             cnt_bit <= cnt_bit + 1'b1;
//             mosi <= DATA_r[DW - cnt_bit - 1'b1];
//         end

//     end
// end
//========================end2==============================//




    
endmodule       

//*************** END OF LINE ********************//