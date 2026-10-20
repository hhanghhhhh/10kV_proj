//************************************************//
//FILE NAME:        DAC_CTRL.V
//DESCRIPTION:      DAC_CTRL模块
//AUTHOR:           hehang
//DATE:             04/17/24
//LOG:              控制DAC输出，只要给定的值与当前值不一样，就启动一次转换
//                  ok_flag为1时代表给定值已经应用成功，可以变化给定了
//                  ok_flag上电后会自动转化一次，因此ok_flag上电3us后为1
//                  一次转换需要2.9us
//************************************************//

//************** Module Define *******************//
module DAC_CTRL
(   
    input                       sys_clk,           //系统时钟  25M
    input                       sys_rst,           //复位信号，低有效
    input[15:0]                 DAC_Set,           //DAC预设值
    
    output                      cs_n,              //片选信号，低有效
    output                      sclk,              //SPI 时钟
    output                      mosi,              //SPI 主机输出
    output  reg                 Ok_Flag            //最后一个不需要逗号
);

//*************** Parameter & Internal Register ****//
// Parameter


// Reg
reg[15:0] dac_current;  //DAC当前输出值

reg DAC_En;
wire DAC_Ok;
reg[15:0] DAC_Data;
//*************** Main Code ***********************//   
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        DAC_Data <= 16'd0;
        DAC_En <= 1'b0;
    end
    else begin
        if(DAC_Ok == 1'b1) begin
            if(DAC_Set != dac_current) begin
                DAC_Data <= DAC_Set;
                DAC_En <= 1'b1;
            end
        end
        else begin
            DAC_En <= 1'b0;
        end
    end
end


//转换过程中，先把当前值赋值过去
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        dac_current <= 16'd1;   //给1是为了使复位后启动一次DAC  使其输出DAC_Set
    end
    else begin
        if(DAC_Ok == 1'b0) begin
            dac_current <= DAC_Data;
        end
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        Ok_Flag <= 1'b0;
    end
    else begin
        if((DAC_Ok == 1'b1) && (dac_current == DAC_Set)) begin
            Ok_Flag <= 1'b1;
        end
        else begin
            Ok_Flag <= 1'b0;
        end
    end
end







DAC_SPI_DRV  dac_spi_inst1
(   
    .sys_clk(sys_clk),         //系统时钟 25M
    .sys_rst(sys_rst),         //复位信号，低有效
    .en(DAC_En),               //启动控制  由0置1开始一次通讯
    .DATA(DAC_Data),           //写入数据 16bit

    .cs_n(cs_n),               //片选信号，低有效
    .sclk(sclk),               //SPI 时钟
    .mosi(mosi),               //SPI 主机输出
    .DAC_OK(DAC_Ok)            //DACOK信号
);











    
endmodule       

//*************** END OF LINE ********************//