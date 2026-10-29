//************************************************//
//FILE NAME:        DATA_REG.V
//DESCRIPTION:      FPGA与DSP数据交互模块
//AUTHOR:           XI LU
//DATE:             20230329
//LOG:              GREATE
//************************************************//

//************** Module Define *******************//
module DATA_REG
(
    input                       sys_clk,        //150M时钟
    input                       sys_rst,
    input      [6:0]            com_dsp_addr,   //与dsp的XA1-7地址线相连
    //dsp与fpga之间连接引脚  
    input                       to_dsp_flag,
    input                       from_dsp_flag,
    //DSP->FPGA，与dsp的XD高16位数据线相连
    input      [31:0]           from_dsp_data,  
    //FPGA->DSP，与dsp的XD低16位数据线相连
    output  reg[31:0]           to_dsp_data,   
    
    //DSP->FPGA的数据
    output  reg[15:0]           vdac_data,          //
    output  reg[15:0]           idac_data,          //
    output  reg[15:0]           sample_num_cur,     //
    output  reg[15:0]           sample_num_vol,     //
    output  reg[15:0]           interval_cnt_cur,   //
    output  reg[15:0]           interval_cnt_vol,   //
    output  reg[15:0]           DAC_riseAmp,        //dac上升的电压/100us
    output  reg[15:0]           DAC_step1Value,     //dac的step1的值 10V-65535
    output  reg[15:0]           DAC_step1Time_H,      //dac的step1的时间，时钟25M
    output  reg[15:0]           DAC_step1Time_L,      //dac的step1的时间，时钟25M
    
    output  reg                 cmd_idac_trigger,   //电流保护DAC输出 trigger
    output  reg                 cmd_vo_coef_sw,     //输出电压采样系数切换
    output  reg                 cmd_vref_sw,        //输出电压给定切换
    output  reg                 cmd_iref_sw,        //限流保护给定切换
    output  reg[15:0]           dsp_cmd,         //
    
    //FPGA->DSP
    input  [15:0]               curadd_H,          //
    input  [15:0]               curadd_M,          //
    input  [15:0]               curadd_L,          //
    input  [31:0]               voladd_32,          //
    input  [31:0]               cur_avg_32,          //
    input  [31:0]               vol_avg_32,          //
    
    input  [15:0]               state_llc_en,           //LLC EN
    input  [15:0]               state_llc_ok,           //LLC OK
    
    input  [15:0]               vol_samp,          //Vo电压采样
    input  [31:0]               cur_samp_32,          //电流采样
    output  reg                 test_last        
);

//*************** Parameter & Internal Register ****//
reg[15:0] CMD_DAC_I_r, CMD_VO_SW_r, CMD_Vref_SW_r, CMD_Iccref_SW_r;
reg[31:0] dsp_cmd_r_32;
wire[15:0] vol_samp_r;
wire[31:0] cur_samp_32_r;
wire[31:0] vol_avg_32_r;
wire[31:0] cur_avg_32_r;

localparam   FPGAVERSION = 2;
//*************** Main Code ***********************//
//FPGA->DSP，将ADC数据读到的数据发送给DSP
reg[31:0] dataw_tmp, tmp_dataW_r[0:1];
reg[6:0] addr_tmp, tmp_addr_r[0:1];
always@(posedge sys_clk) begin
    tmp_addr_r[0] <= com_dsp_addr;
    tmp_addr_r[1] <= tmp_addr_r[0];
    tmp_dataW_r[0] <= from_dsp_data;
    tmp_dataW_r[1] <= tmp_dataW_r[0];
    if(tmp_addr_r[0] == tmp_addr_r[1]) begin
        addr_tmp <= tmp_addr_r[0];
    end
    if(tmp_dataW_r[0] == tmp_dataW_r[1]) begin
        dataw_tmp <= tmp_dataW_r[0];
    end
end

always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        to_dsp_data <= 32'd0;
    end
    else begin
        if(to_dsp_flag == 1'b0) begin
            case(addr_tmp)
                7'd0: to_dsp_data <= {16'd0, curadd_H};
                7'd1: to_dsp_data <= {16'd0, curadd_M};
                7'd2: to_dsp_data <= {16'd0, curadd_L};

                7'd4: to_dsp_data <= {16'd0, vol_samp_r};
                7'd5: to_dsp_data <= voladd_32;
                
                7'd7: to_dsp_data <= cur_samp_32_r;
                7'd8: to_dsp_data <= cur_avg_32_r;
                7'd9: to_dsp_data <= vol_avg_32_r;
                
                7'd101: to_dsp_data <= {16'd0, state_llc_en};
                7'd102: to_dsp_data <= {16'd0, state_llc_ok};
                7'd85:  to_dsp_data <= {16'd0, FPGAVERSION};   //FPGAversion
                7'd127: to_dsp_data <= {16'd0, 16'd456};   //test
            endcase
        end
    end
end


reg[6:0] tmp_addr;
reg[31:0] tmp_dataW;
//DSP->FPGA，读取DSP发送过来的数据
always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        vdac_data <= 16'd0;
        idac_data <= 16'd0;
        sample_num_cur <= 16'd0;
        sample_num_vol <= 16'd0;
        interval_cnt_cur <= 16'd0;
        interval_cnt_vol <= 16'd0;
        DAC_riseAmp <= 16'd6;
        DAC_step1Value <= 16'd13106;
        DAC_step1Time_H <= 16'd6;
        DAC_step1Time_L <= 16'd31784;
        CMD_DAC_I_r <= 16'd2222;
        CMD_VO_SW_r <= 16'd2222;
        CMD_Vref_SW_r <= 16'd1111;
        CMD_Iccref_SW_r <= 16'd1111;
        dsp_cmd_r_32 <= 32'd0;
    end
    else begin
        if(from_dsp_flag == 1'b0)
        begin
            tmp_dataW <= dataw_tmp;
            tmp_addr <= addr_tmp;
        end
        else begin
            case(tmp_addr)
                7'd0: vdac_data <= tmp_dataW[15:0];
                7'd1: idac_data <= tmp_dataW[15:0];
                7'd2: sample_num_cur <= tmp_dataW[15:0];
                7'd3: interval_cnt_cur <= tmp_dataW[15:0];
                7'd6: DAC_riseAmp <= tmp_dataW[15:0];
                7'd7: DAC_step1Value <= tmp_dataW[15:0];
                7'd8: DAC_step1Time_H <= tmp_dataW[15:0];
                7'd9: DAC_step1Time_L <= tmp_dataW[15:0];
                7'd10: dsp_cmd_r_32 <= tmp_dataW;
                
                7'd13: sample_num_vol <= tmp_dataW[15:0];
                7'd14: interval_cnt_vol <= tmp_dataW[15:0];
                
                7'd50: CMD_DAC_I_r <= tmp_dataW[15:0];
                7'd52: CMD_VO_SW_r <= tmp_dataW[15:0];
                7'd53: CMD_Vref_SW_r <= tmp_dataW[15:0];
                7'd54: CMD_Iccref_SW_r <= tmp_dataW[15:0];
            endcase
        end
    end
end


always@(posedge sys_clk) begin
    if(dsp_cmd_r_32[31:16] == dsp_cmd_r_32[15:0]) begin
        dsp_cmd <= dsp_cmd_r_32[15:0];
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cmd_idac_trigger <= 1'b0;
    end
    else begin
        if(CMD_DAC_I_r == 16'd1111) begin
            cmd_idac_trigger <= 1'b1;
        end
        else if(CMD_DAC_I_r == 16'd2222) begin
            cmd_idac_trigger <= 1'b0;
        end
        else begin
            cmd_idac_trigger <= cmd_idac_trigger;
        end
    end
end



always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cmd_vo_coef_sw <= 1'b0;
    end
    else begin
        if(CMD_VO_SW_r == 16'd1111) begin
            cmd_vo_coef_sw <= 1'b1;
        end
        else if(CMD_VO_SW_r == 16'd2222) begin
            cmd_vo_coef_sw <= 1'b0;
        end
        else begin
            cmd_vo_coef_sw <= cmd_vo_coef_sw;
        end
        
    end
end




always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cmd_vref_sw <= 1'b0;
    end
    else begin
        if(CMD_Vref_SW_r == 16'd1111) begin
            cmd_vref_sw <= 1'b1;
        end
        else if(CMD_Vref_SW_r == 16'd2222) begin
            cmd_vref_sw <= 1'b0;
        end
        else begin
            cmd_vref_sw <= cmd_vref_sw;
        end
        
    end
end



always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        cmd_iref_sw <= 1'b0;
    end
    else begin
        if(CMD_Iccref_SW_r == 16'd1111) begin
            cmd_iref_sw <= 1'b1;
        end
        else if(CMD_Iccref_SW_r == 16'd2222) begin
            cmd_iref_sw <= 1'b0;
        end
        else begin
            cmd_iref_sw <= cmd_iref_sw;
        end
        
    end
end




async_double_flopping
#(
    .DATA_WIDTH      (32)
)
vol_avg_inst1
(   
    .clk_data_out    (sys_clk       ),  
    .sys_rst         (sys_rst       ),
    .data_in         (vol_avg_32    ),
    .data_out        (vol_avg_32_r  )      
);
async_double_flopping
#(
    .DATA_WIDTH      (32)
)
cur_avg_inst1
(   
    .clk_data_out    (sys_clk       ),  
    .sys_rst         (sys_rst       ),
    .data_in         (cur_avg_32    ),
    .data_out        (cur_avg_32_r  )      
);
async_double_flopping
#(
    .DATA_WIDTH      (16)
)
vol_samp_inst1
(   
    .clk_data_out    (sys_clk       ),  
    .sys_rst         (sys_rst       ),
    .data_in         (vol_samp      ),
    .data_out        (vol_samp_r    )      
);
async_double_flopping
#(
    .DATA_WIDTH      (32)
)
cur_samp_inst1
(   
    .clk_data_out    (sys_clk       ),  
    .sys_rst         (sys_rst       ),
    .data_in         (cur_samp_32   ),
    .data_out        (cur_samp_32_r )      
);


endmodule

//*************** END OF LINE ********************//    
    
    