/************************************************
//FILE NAME:        median_filter.v
//DESCRIPTION:      median_filter
//AUTHOR:           hehang
//DATE:             25/10/15
//LOG:              中值滤波，7个点
************************************************/
//************** Module Define *******************//
module median_filter #(
    parameter DATA_WIDTH = 24
)(   
    input                                 sys_clk             ,  // xxM
    input                                 sys_rst_n           ,
    input                                 din_valid           ,  // 输入数据有效信号
    input       signed [DATA_WIDTH-1:0]   din                 ,  // 输入数据
    output reg                            dout_valid          ,  // 输出数据有效信号
    output reg         [DATA_WIDTH-1:0]   dout                   // 输出中值
);

//*************** Parameter & Internal Register ****//
// parameter


// reg
reg   signed  [DATA_WIDTH-1:0]       shift_reg [0:6]   ;
reg           [2:0]                  sample_cnt        ;  // 记录已有多少个样本（0~7）
wire  signed  [DATA_WIDTH-1:0]       s[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       a[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       b[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       c[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       d[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       e[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       f[0:6]            ;
wire  signed  [DATA_WIDTH-1:0]       median_val        ;
reg   signed  [DATA_WIDTH-1:0]       median_val_r1     ;
reg   signed  [DATA_WIDTH-1:0]       median_val_r2     ;
reg                                  din_valid_r[0:5]  ; 

// assign

//*************** Main Code ***********************//
// 移位寄存器保存最近7个采样
integer i;
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        for(i=0; i<7; i=i+1) begin
            shift_reg[i] <= 0;
        end
        sample_cnt <= 0;
    end else if(din_valid) begin
        shift_reg[0] <= din;
        for(i=1; i<7; i=i+1) begin
            shift_reg[i] <= shift_reg[i-1];
        end
        if(sample_cnt < 7) begin
            sample_cnt <= sample_cnt + 1;
        end
    end
end

// 中值计算逻辑
// 比较-交换函数 
function [2*DATA_WIDTH-1:0] sort2;
    input signed [DATA_WIDTH-1:0] a, b;
    begin
        if(a < b)
            sort2 = {a, b}; // {小, 大}
        else
            sort2 = {b, a};
    end
endfunction

assign  s[0]        = shift_reg[0]    ;
assign  s[1]        = shift_reg[1]    ;
assign  s[2]        = shift_reg[2]    ;
assign  s[3]        = shift_reg[3]    ;
assign  s[4]        = shift_reg[4]    ;
assign  s[5]        = shift_reg[5]    ;
assign  s[6]        = shift_reg[6]    ;
// 固定比较网络求中值 (median of 7)
// 第一层比较交换
assign  {a[0],a[1]} = sort2(s[0],s[1]);
assign  {a[2],a[3]} = sort2(s[2],s[3]);
assign  {a[4],a[5]} = sort2(s[4],s[5]);
assign  a[6] = s[6];
// 第二层
assign  {b[0],b[2]} = sort2(a[0],a[2]);
assign  {b[1],b[3]} = sort2(a[1],a[3]);
assign  {b[4],b[6]} = sort2(a[4],a[6]);
assign  b[5] = a[5];
// 第三层
assign  {c[1],c[2]} = sort2(b[1],b[2]);
assign  {c[5],c[6]} = sort2(b[5],b[6]);
assign  c[0] = b[0];
assign  c[3] = b[3];
assign  c[4] = b[4];
// 第四层
assign  {d[0],d[4]} = sort2(c[0],c[4]);
assign  {d[1],d[5]} = sort2(c[1],c[5]);
assign  {d[2],d[6]} = sort2(c[2],c[6]);
assign  d[3] = c[3];
// 第五层
assign  {e[2],e[4]} = sort2(d[2],d[4]);
assign  {e[3],e[5]} = sort2(d[3],d[5]);
assign  e[0] = d[0];
assign  e[1] = d[1];
assign  e[6] = d[6];
// 第六层
assign  {f[1],f[2]} = sort2(e[1],e[2]);
assign  {f[3],f[4]} = sort2(e[3],e[4]);
assign  {f[5],f[6]} = sort2(e[5],e[6]);
assign  f[0] = e[0];
// 中值输出（第4小）
assign  median_val = f[3]       ;

always @(posedge sys_clk) begin
    median_val_r1 <= median_val;
    median_val_r2 <= median_val_r1;
    din_valid_r[0] <= din_valid;
    din_valid_r[1] <= din_valid_r[0];
    din_valid_r[2] <= din_valid_r[1];
    din_valid_r[3] <= din_valid_r[2];
    din_valid_r[4] <= din_valid_r[3];
    din_valid_r[5] <= din_valid_r[4];
end

// 输出时序控制：当数据准备好后输出有效
always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        dout_valid      <= 1'b0;
        dout            <= {DATA_WIDTH{1'b0}};
    end else begin
        // 当样本数量足够（>=7）并且当前输入有效时，输出才有效
        if(sample_cnt >= 7) begin
            if (din_valid_r[5]) begin
                dout_valid <= 1'b1;
                dout <= median_val_r2;
            end
            else begin
                dout_valid <= 1'b0;
            end
        end
    end
end







endmodule
//*************** END OF LINE ********************//