# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "CSR_ADDR_SIZE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "CSR_ID_WIDTH" -parent ${Page_0}


}

proc update_PARAM_VALUE.CSR_ADDR_SIZE { PARAM_VALUE.CSR_ADDR_SIZE } {
	# Procedure called to update CSR_ADDR_SIZE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CSR_ADDR_SIZE { PARAM_VALUE.CSR_ADDR_SIZE } {
	# Procedure called to validate CSR_ADDR_SIZE
	return true
}

proc update_PARAM_VALUE.CSR_ID_WIDTH { PARAM_VALUE.CSR_ID_WIDTH } {
	# Procedure called to update CSR_ID_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CSR_ID_WIDTH { PARAM_VALUE.CSR_ID_WIDTH } {
	# Procedure called to validate CSR_ID_WIDTH
	return true
}


proc update_MODELPARAM_VALUE.CSR_ID_WIDTH { MODELPARAM_VALUE.CSR_ID_WIDTH PARAM_VALUE.CSR_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CSR_ID_WIDTH}] ${MODELPARAM_VALUE.CSR_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.CSR_ADDR_SIZE { MODELPARAM_VALUE.CSR_ADDR_SIZE PARAM_VALUE.CSR_ADDR_SIZE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CSR_ADDR_SIZE}] ${MODELPARAM_VALUE.CSR_ADDR_SIZE}
}

