/*
 * com_pl_reg.c
 *
 *  Created on: 2025Äê7ÔÂ2ÈÕ
 *      Author: m
 */
#include "COMMON.h"

ST_TOPL st_topl;
ST_FROMPL st_frompl;

void InitTaskCom(void)
{
	memset(&st_topl, 0 ,sizeof(ST_TOPL));
	memset(&st_frompl, 0 ,sizeof(ST_FROMPL));

	st_topl.dac.bit.vol = 10;
	st_topl.dac.bit.cur = 10;

	st_topl.cmd.bit.outoff = 1;
}



void TaskComToPl(void)
{
	WritePlReg(0, st_topl.cmd.u32);
	WritePlReg(1, st_topl.dac.u32);
	WritePlReg(2, st_topl.samp.u32);
	WritePlReg(3, st_topl.cmd2.u32);
	WritePlReg(4, st_topl.para.u32);
	WritePlReg(5, st_topl.para2.u32);
	WritePlReg(6, st_topl.pl_cmd.u32);
	WritePlReg(7, st_topl.pl_cmd2.u32);
	WritePlReg(8, st_topl.test_ddr3.u32);
	WritePlReg(9, st_topl.wave_para.u32);
}

void TaskComFromPl(void)
{
	st_frompl.vol_avg_ad 		= ReadPlReg(1);
	st_frompl.cur_avg_ad 		= ReadPlReg(2);
	st_frompl.vol_ad 			= ReadPlReg(3);
	st_frompl.cur_ad 			= ReadPlReg(4);
	st_frompl.state.u32 		= ReadPlReg(5);
	st_frompl.slowrx_err_cnt 	= ReadPlReg(8);
	st_frompl.pl_data.u32 		= ReadPlReg(10);
	st_frompl.pl_data2.u32 		= ReadPlReg(11);
	st_frompl.test_cnt.u32 		= ReadPlReg(12);
	st_frompl.test_awaddr		= ReadPlReg(13);

	mgmd_stSCIRx.rx_err_cnt.u32 = st_frompl.slowrx_err_cnt;
}

