module  posedge_detect
(
input    sys_clk,
input    rst_n,
input    signal,
input    signal_default,
output   signal_pedge
);

reg      signal_z0;
reg      signal_z1;

always@(posedge sys_clk, negedge rst_n)begin
	if(!rst_n) begin
		signal_z0 <= signal_default;
		signal_z1 <= signal_default;
	end
	else begin
		signal_z1 <= signal_z0;
		signal_z0 <= signal;

	end
end
	

assign signal_pedge = (!signal_z1) && (signal_z0);

endmodule
