//************************************************//
//FILE NAME:        SIGNAL_CTRL.V
//DESCRIPTION:      SIGNAL_CTRL模块
//AUTHOR:           hehang
//DATE:             07/27/23
//LOG:              控制信号逻辑
//************************************************//

//************** Module Define *******************//
module SIGNAL_CTRL
(   
    input                       sys_clk,           //系统时钟  25M
    input                       sys_rst,           //复位信号，低有效
    input                       en,                //模块使能信号，高有效
    input                       in_pfc_ok,             //  0有效
    input                       in_llc_ok,             //
    output  reg                 pfc_en,             //  0使能
    output  reg                 llc_en              //  1使能

);

//*************** Parameter & Internal Register ****//
// Parameter
parameter  PFC_DELAY = 32'd10;   //0s
parameter  LLC_DELAY = 32'd25_000_000;   //1s

// Reg


//*************** Main Code ***********************//   
//上电1s后拉低PFC_EN
reg[31:0] cnt_pfcen;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_pfcen <= 32'd0;
        pfc_en <= 1'b1;
    end
    else begin
        if(en == 1'b1) begin
            if(cnt_pfcen == PFC_DELAY) begin
                cnt_pfcen <= cnt_pfcen;
                pfc_en <= 1'b0;
            end
            else begin
                cnt_pfcen <= cnt_pfcen + 32'd1;
                pfc_en <= 1'b1;
            end
        end
        else begin
            // initial regs when the module is disabled 
            pfc_en <= 1'b1;
        end
    end
end


//in_pfc_ok == 0后延时再使能LLC_EN
reg[31:0] cnt_llcen;
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cnt_llcen <= 32'd0;
    end
    else begin
        if(en == 1'b1) begin
            if((in_pfc_ok == 1'b0) && (pfc_en == 1'b0)) begin
                if(cnt_llcen == LLC_DELAY) begin  //100ms
                    cnt_llcen <= cnt_llcen;
                end
                else begin
                    cnt_llcen <= cnt_llcen + 32'd1;
                end
            end
            else begin
                cnt_llcen <= 32'd0;
            end
        end
        else begin
            // initial regs when the module is disabled 
            cnt_llcen <= 32'd0;
        end
    end
end



always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        llc_en <= 1'b0;
    end
    else begin
        if(en == 1'b1) begin
            if(cnt_llcen == LLC_DELAY) begin
                llc_en <= 1'b1;
            end
            else begin
                llc_en <= 1'b0;
            end
        end
        else begin
            // initial regs when the module is disabled 
            llc_en <= 1'b0;
        end
    end
end

endmodule       

//*************** END OF LINE ********************//