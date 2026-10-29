module DFF_R #(
    parameter       DWIDTH  = 1
) (
    input                   CLK           ,
    input                   RSTN          ,
    input     [DWIDTH-1:0]  D              ,
    output    [DWIDTH-1:0]  Q
);
reg [DWIDTH-1:0]  Q_reg;
assign Q = Q_reg;

always @(posedge CLK or negedge RSTN) begin
    if(!RSTN)   Q_reg <= 0;
    else         Q_reg <= D;
end
    
endmodule
