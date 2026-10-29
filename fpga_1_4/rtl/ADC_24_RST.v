//************************************************//
//FILE NAME:        ADC_24_RST.V
//DESCRIPTION:      ADC_24_RST
//AUTHOR:           hehang
//DATE:             04/18/24
//LOG:              24bit ADC reset and en
//************************************************//

//************** Module Define *******************//
module ADC_24_RST
(   
    input                       sys_clk,           //系统时钟  25M
    input                       sys_rst,           //复位信号，低有效
    input                       en,                //模块使能信号，高有效


    output  reg                 rst_n,          //
    output  reg                 adc_sample_en  //开始转换 //最后一个不需要逗号
);

//*************** Parameter & Internal Register ****//
// Parameter


// Reg
reg[31:0] cnt_rst;

//*************** Main Code ***********************//   
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_rst <= 32'd0;
    end
    else begin
        if(en == 1'b1) begin
            if(cnt_rst < 32'd25_000_000) begin  //总计数25s
                cnt_rst <= cnt_rst + 32'd1;
            end
            else begin
                cnt_rst <= cnt_rst;
            end
        end
        else begin
            // initial regs when the module is disabled 
            cnt_rst <= 32'd0;
        end
    end
end



always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        rst_n <= 1'b1;
        adc_sample_en <= 1'b0;
    end
    else begin
        if(en == 1'b1) begin
            if(cnt_rst == 32'd1) begin
                rst_n <= 1'b1;
                adc_sample_en <= 1'b0;
            end
            else if(cnt_rst == 32'd75_000) begin  //3ms
                rst_n <= 1'b0;
                adc_sample_en <= 1'b0;
            end
            else if(cnt_rst == 32'd77_500) begin  //100us 2500
                rst_n <= 1'b1;
                adc_sample_en <= 1'b0;
            end
            else if(cnt_rst == 32'd102_500) begin  //1ms 25000  
                rst_n <= 1'b1;
                adc_sample_en <= 1'b1;
            end
        end
        else begin
            // initial regs when disable module
            rst_n <= 1'b1;
            adc_sample_en <= 1'b0;
        end
    end
end








    
endmodule       

//*************** END OF LINE ********************//