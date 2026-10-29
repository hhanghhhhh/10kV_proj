//`include "defines.vh"
module AXI_LITE_S #(
  parameter CSR_ID_WIDTH  = 1,
  parameter CSR_ADDR_SIZE = 12
)(
// AXI_Lite Slave
  input                       axilRST_rstn_I,   
  input                       S_AXI_L_ACLK_I,
  input [2:0]                 M2L_AWPROT_I,
  input [31:0]                M2L_AWADDR_I,
  input [CSR_ID_WIDTH-1:0]    M2L_AWID_I,
  input                       M2L_AWVALID_I,
  input [31:0]                M2L_WDATA_I,
  input [3:0]                 M2L_WSTRB_I,
  input                       M2L_WVALID_I,
  input                       M2L_BREADY_I,
  input [2:0]                 M2L_ARPROT_I,
  input [31:0]                M2L_ARADDR_I,
  input [CSR_ID_WIDTH-1:0]    M2L_ARID_I,
  input                       M2L_ARVALID_I,
  input                       M2L_RREADY_I,
// AXI_Lite Slave

// from REG
  input                       REG_itf_wr_done_i,
  input                       REG_bresp_slverr_i,
  input                       REG_itf_rd_done_i,
  input                       REG_rresp_slverr_i,
  input [31:0]                REG_itf_rdata_i,
// from REG

// AXI_Lite_Slave
  output                      LIT_AWREADY_O,
  output                      LIT_WREADY_O,
  output                      LIT_BVALID_O,
  output [CSR_ID_WIDTH-1:0]   LIT_BID_O,
  output [1:0]                LIT_BRESP_O,
  output                      LIT_ARREADY_O,
  output                      LIT_RVALID_O,
  output [31:0]               LIT_RDATA_O,
  output                      LIT_RLAST_O,
  output [CSR_ID_WIDTH-1:0]   LIT_RID_O,
  output [1:0]                LIT_RRESP_O,
// AXI_Lite Slave

// to REG
  output                      LIT_memwr_en_o,
  output [CSR_ADDR_SIZE-1:0]  LIT_memwr_addr_o,
  output [31:0]               LIT_memwr_dat_o,
  output [3:0]                LIT_memwr_strb_o,
  output                      LIT_memrd_en_o,
  output [CSR_ADDR_SIZE-1:0]  LIT_memrd_addr_o
// to REG
);
wire w_reg_wr_done_nxt;
wire w_reg_rd_done_nxt;
wire [31:0] w_reg_data_nxt;
wire w_rd_req_accept_nxt;
wire w_wr_req_accept_nxt;
wire w_wr_data_accept_nxt;
wire w_rd_rsp_accept_nxt;
wire w_wr_rsp_accept_nxt;
wire w_axi_congestion_nxt;
wire w_axi_awready_en_inv;
wire w_axi_awready_en_upd;
wire w_axi_awready_en_nxt;
wire axi_awready_en;
wire w_axi_busy_upd;
wire w_axi_busy_nxt;
wire axi_busy;
wire w_wready_upd;
wire w_bvalid_upd;
wire [1:0] w_bresp_nxt;
wire w_rvalid_upd;
wire [1:0] w_rresp_nxt;

assign w_reg_wr_done_nxt    = REG_itf_wr_done_i;
assign w_reg_rd_done_nxt    = REG_itf_rd_done_i;
assign w_reg_data_nxt       = REG_itf_rdata_i;					 
assign w_rd_req_accept_nxt  = M2L_ARVALID_I & LIT_ARREADY_O;
assign w_wr_req_accept_nxt  = M2L_AWVALID_I & LIT_AWREADY_O;
assign w_wr_data_accept_nxt = M2L_WVALID_I & LIT_WREADY_O;
assign w_rd_rsp_accept_nxt  = LIT_RVALID_O & M2L_RREADY_I;
assign w_wr_rsp_accept_nxt  = LIT_BVALID_O & M2L_BREADY_I;
assign w_axi_congestion_nxt = M2L_ARVALID_I & M2L_AWVALID_I;

assign w_axi_awready_en_inv = w_axi_congestion_nxt & (LIT_ARREADY_O | LIT_AWREADY_O);
assign w_axi_awready_en_upd = w_axi_awready_en_inv | w_rd_req_accept_nxt |  w_wr_req_accept_nxt;
assign w_axi_awready_en_nxt = w_axi_awready_en_inv ? ~axi_awready_en : 1'b0;
DFF_R_L #(.DWIDTH(1)) U_axi_awready_en (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_axi_awready_en_upd), .D(w_axi_awready_en_nxt), .Q(axi_awready_en));

assign w_axi_busy_upd = w_rd_req_accept_nxt | w_wr_req_accept_nxt | w_rd_rsp_accept_nxt | w_wr_rsp_accept_nxt;						
assign w_axi_busy_nxt = w_rd_rsp_accept_nxt || w_wr_rsp_accept_nxt ? 1'b0 : 1'b1;
DFF_R_L #(.DWIDTH(1)) U_axi_busy (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_axi_busy_upd), .D(w_axi_busy_nxt), .Q(axi_busy));

assign LIT_ARREADY_O = ~( axi_busy | (w_axi_congestion_nxt & axi_awready_en));

assign LIT_AWREADY_O = ~(axi_busy | (w_axi_congestion_nxt & ~axi_awready_en));

// @ axi write data
// Assert WREADY after write address accepted
assign w_wready_upd = w_wr_req_accept_nxt | w_wr_data_accept_nxt;
DFF_R_L #(.DWIDTH(1)) U_wready (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_wready_upd), .D(w_wr_req_accept_nxt), .Q(LIT_WREADY_O));
// @ axi write data

// @ axi write response
assign w_bvalid_upd = w_reg_wr_done_nxt | w_wr_rsp_accept_nxt;
DFF_R_L #(.DWIDTH(1)) U_bvalid (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_bvalid_upd), .D(w_reg_wr_done_nxt), .Q(LIT_BVALID_O));
DFF_R_L #(.DWIDTH(CSR_ID_WIDTH)) U_bid (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_wr_req_accept_nxt), .D(M2L_AWID_I), .Q(LIT_BID_O));

assign w_bresp_nxt = REG_bresp_slverr_i ? 2'b10 : 2'b00;
DFF_R_L #(.DWIDTH(2)) U_bresp (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_reg_rd_done_nxt), .D(w_bresp_nxt), .Q(LIT_BRESP_O));

// @ axi write response

// @ axi read data
assign w_rvalid_upd = w_reg_rd_done_nxt | w_rd_rsp_accept_nxt;
DFF_R_L #(.DWIDTH(1)) U_rvalid (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_rvalid_upd), .D(w_reg_rd_done_nxt), .Q(LIT_RVALID_O));
DFF_R_L #(.DWIDTH(32)) U_rdata (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_reg_rd_done_nxt), .D(w_reg_data_nxt), .Q(LIT_RDATA_O));
DFF_R_L #(.DWIDTH(CSR_ID_WIDTH)) U_rid (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_rd_req_accept_nxt), .D(M2L_ARID_I), .Q(LIT_RID_O));

assign w_rresp_nxt = REG_rresp_slverr_i ? 2'b10 : 2'b00;
DFF_R_L #(.DWIDTH(2)) U_rresp (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_reg_rd_done_nxt), .D(w_rresp_nxt), .Q(LIT_RRESP_O));

assign LIT_RLAST_O = 1'b1;
// @ axi read data

// @ Issue write request
// latch address -> latch data -> to do write
DFF_R   #(.DWIDTH(1)) U_memwr_en (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .D(w_wr_data_accept_nxt), .Q(LIT_memwr_en_o));
DFF_R_L #(.DWIDTH(CSR_ADDR_SIZE)) U_memwr_addr (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_wr_req_accept_nxt), .D(M2L_AWADDR_I[CSR_ADDR_SIZE-1:0]), .Q(LIT_memwr_addr_o));
DFF_R_L #(.DWIDTH(32)) U_memwr_dat (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_wr_data_accept_nxt), .D(M2L_WDATA_I), .Q(LIT_memwr_dat_o));
DFF_R_L #(.DWIDTH(4)) U_memwr_strb (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_wr_data_accept_nxt), .D(M2L_WSTRB_I), .Q(LIT_memwr_strb_o));
// @ Issue write request

// @ Issue read request
DFF_R #(.DWIDTH(1)) U_memrd_en (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .D(w_rd_req_accept_nxt), .Q(LIT_memrd_en_o));
DFF_R_L #(.DWIDTH(CSR_ADDR_SIZE)) U_memrd_addr (.RSTN(axilRST_rstn_I), .CLK(S_AXI_L_ACLK_I), .EN(w_rd_req_accept_nxt), .D(M2L_ARADDR_I[CSR_ADDR_SIZE-1:0]), .Q(LIT_memrd_addr_o));
// @ Issue read request
endmodule