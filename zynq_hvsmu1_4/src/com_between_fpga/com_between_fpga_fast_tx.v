/************************************************
//FILE NAME:        com_between_fpga_fast_tx.v
//DESCRIPTION:      com_between_fpga_fast_tx
//AUTHOR:           hehang
//DATE:             25/06/20
//LOG:              
************************************************/
//************** Module Define *******************//
module com_between_fpga_fast_tx
(   
    input                       sys_clk             ,  // xxM
    input                       sys_rst_n           ,
    input       [31:0]                          ,

    output  reg                 sclk                ,
    output  reg                 csn                 ,
    output  reg [3:0]           data                ,
);

//*************** Parameter & Internal Register ****//
// parameter


// reg


//*************** Main Code ***********************//
always@(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n) begin
        
    end else begin
        
    end
end










endmodule
//*************** END OF LINE ********************//