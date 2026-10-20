//************************************************//
//FILE NAME:        adjustable_duty_complementary_pwm.V
//DESCRIPTION:      Generates complementary PWM waves with adjustable duty cycle
//AUTHOR:           hehang
//DATE:             20250409
//LOG:              
//************************************************//
module adjustable_duty_complementary_pwm
#(
parameter PERIOD_WIDTH           =  10                      ,
parameter PERIOD_CNT             =  400                     ,
parameter DEADTIME_CNT           =  2                       
)
(
input                                   sys_clk             ,
input                                   sys_rst             ,
input                                   enable              ,
input          [PERIOD_WIDTH - 1:0]     duty_cycle          ,
output reg                              gate1               ,
output reg                              gate2           
);


reg         [PERIOD_WIDTH - 1:0]   duty_cycle_limit;
wire        [PERIOD_WIDTH - 1:0]   duty_cycle_r;
reg         [PERIOD_WIDTH - 1:0]   period_cnt;
wire        [PERIOD_WIDTH - 1:0]   cnt_gate1_on;
wire        [PERIOD_WIDTH - 1:0]   cnt_gate1_off;
wire        [PERIOD_WIDTH - 1:0]   cnt_gate2_on;
wire        [PERIOD_WIDTH - 1:0]   cnt_gate2_off;
wire                               en_pedge;


assign  cnt_gate1_on = 0;
assign  cnt_gate1_off = duty_cycle_limit + 1'b1 - DEADTIME_CNT;
assign  cnt_gate2_on = duty_cycle_limit;
assign  cnt_gate2_off = PERIOD_CNT + 1'b1 - DEADTIME_CNT;


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
        duty_cycle_limit <= (PERIOD_CNT >> 1);
    end 
    else if(period_cnt == 1 || en_pedge)begin
        if(duty_cycle_r > PERIOD_CNT) begin
            duty_cycle_limit <= PERIOD_CNT;
        end
        else if(duty_cycle_r < DEADTIME_CNT) begin
            duty_cycle_limit <= DEADTIME_CNT;
        end
        else begin
            duty_cycle_limit <= duty_cycle_r;
        end
    end
    else begin
        duty_cycle_limit <= duty_cycle_limit;
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
        period_cnt <= {PERIOD_WIDTH{1'b0}};
    end 
    else if(enable)begin
        if(period_cnt < PERIOD_CNT - 1'b1)begin
            period_cnt <= period_cnt + 1'b1;
        end
        else begin
            period_cnt <= {PERIOD_WIDTH{1'b0}};
        end
    end
    else begin
        period_cnt <= {PERIOD_WIDTH{1'b0}};
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        gate1 <= 1'b0;
    end
    else begin
        if(gate2 == 1'b0 && period_cnt > cnt_gate1_on && period_cnt < cnt_gate1_off) begin
            gate1 <= 1'b1;
        end
        else begin
            gate1 <= 1'b0;
        end
    end
end


always@(posedge sys_clk or negedge sys_rst) begin
    if(!sys_rst) begin
    //initial regs
        gate2 <= 1'b0;
    end
    else begin
        if(period_cnt > cnt_gate2_on && period_cnt < cnt_gate2_off) begin
            gate2 <= 1'b1;
        end
        else begin
            gate2 <= 1'b0;
        end
    end
end




posedge_detect       llc_work_en_pedge_detect
(
.sys_clk              (sys_clk          ),
.rst_n                (rst_n            ),
.signal               (enable           ),
.signal_default       (1'b0             ),
.signal_pedge         (en_pedge         )
);

async_double_flopping 
#(
    .DATA_WIDTH      (PERIOD_WIDTH      )
)
duty_inst
(   
    .clk_data_out    (sys_clk           ),  
    .sys_rst         (sys_rst           ),
    .data_in         (duty_cycle        ),
    .data_out        (duty_cycle_r      )      
);


endmodule
