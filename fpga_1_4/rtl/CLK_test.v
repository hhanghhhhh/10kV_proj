//************************************************//
//FILE NAME:        CLK_test.V
//DESCRIPTION:      CLK_test模块
//AUTHOR:           hehang
//DATE:             05/17/23
//LOG:              
//************************************************//

//************** Module Define *******************//
module CLK_test
(   
    input             sys_clk,        //系统时钟  25M
    input             sys_rst,        //复位信号，低有效

    output  reg       clk
);

//*************** Parameter & Internal Register ****//
// Parameter
parameter CLK_CNT = 16'd100;
parameter CLK_CNT_DIV2 = CLK_CNT/2;
// Reg
reg[15:0] cnt_clk;
//*************** Main Code ***********************//   
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        clk <= 1'b0;
        cnt_clk <= 16'd0;
    end
    else begin
        if(cnt_clk == CLK_CNT_DIV2) begin
            clk <= 1'b1;
            cnt_clk <= cnt_clk + 16'd1;
        end
        else if(cnt_clk == CLK_CNT - 16'd1) begin
            clk <= 1'b0;
            cnt_clk <= 'd0;
        end
        else begin
            clk <= clk;
            cnt_clk <= cnt_clk + 16'd1;
        end
    end
end


endmodule       

//*************** END OF LINE ********************//