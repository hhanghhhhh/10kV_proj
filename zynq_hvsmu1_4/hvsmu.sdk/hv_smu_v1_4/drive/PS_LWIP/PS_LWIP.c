/*
 * PS_LWIP.c
 *
 *  Created on: 2025年6月18日
 *      Author: m
 */

#include "PS_LWIP.h"
#include "platform_config.h"
#include "COMMON.h"
#include "netif/xemacpsif.h"
#include "xemacps.h"
#define DEFAULT_IP_ADDRESS	"192.168.1.10"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"

struct netif server_netif;

void start_application(void);


Uint16 hh_test_read_last = 0;
Uint16 hh_test_read_new = 0;
Uint16 hh_test_read_0x01 = 0;
Uint16 hh_test_read_0x11 = 0;


Uint16 hh_test_read_0xA3[20];
Uint16 hh_start_addr = 0xa3;
void ReadPhyRegMain(void)
{
	Uint16 i = 0;
	struct netif *netif;
	netif = &server_netif;

	struct xemac_s *xemac = (struct xemac_s *)(netif->state);
	xemacpsif_s *xemacs = (xemacpsif_s *)(xemac->state);
	XEmacPs *xemacp = &xemacs->emacps;

	XEmacPs_PhyRead(xemacp, 4 , 0x01, &hh_test_read_0x01);
	XEmacPs_PhyRead(xemacp, 4 , 0x01, &hh_test_read_0x01);
	XEmacPs_PhyRead(xemacp, 4 , 0x11, &hh_test_read_0x11);

	hh_start_addr = 0xa3;
	for(i = 0; i < 20; i++)
	{
		XEmacPs_PhyWrite(xemacp, 4 , 0x1e, hh_start_addr + i);
		XEmacPs_PhyRead(xemacp, 4 , 0x1f, &hh_test_read_0xA3[i]);
	}

}


void ReadPhyReg(void)
{
	struct netif *netif;
	netif = &server_netif;

	struct xemac_s *xemac = (struct xemac_s *)(netif->state);
	xemacpsif_s *xemacs = (xemacpsif_s *)(xemac->state);
	XEmacPs *xemacp = &xemacs->emacps;


//	上电硬复位后，初始化 PHY 相关寄存器配置，优化 UTP 接口性能。
//	write utp ext regOx52:0x231d
//	write utp ext regOx51: 0x04a9
//	write utp ext regOx57:0x274c
//	write utp mii regOx0:0x9140
//	#优化 UTP 长线性能
//	#优化 UTP 电口指标性能
//	#优化 UTP 电口指标性能
//	#软复位
//	在 RGMI 电压为 1.8V 时，需要注意增加如下两项配置:
//	write utp ext reg0xa010: 0xabff
//	write utp ext reg0xa003: 0xf0
//	#优化 1.8V RGMI 性能
//	#优化 1.8V RGMI 性能

//	// write utp ext regOx52:0x231d
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0x52);
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0x231d);
//	//	write utp ext regOx51: 0x04a9
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0x51);
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0x04a9);
//	//	write utp ext regOx57:0x274c
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0x57);
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0x274c);
//	//	write utp mii regOx0:0x9140
//	XEmacPs_PhyWrite(xemacp, 4 , 0x0, 0x9140);
//
//	// delay 10ms
//	usleep(10000);
//
//	//	write utp ext regOxa010: 0xabff
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xa010);
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0xabff);
//
//	//read   ext regOxa003: 0xf0  --test
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xa003);
//	XEmacPs_PhyRead(xemacp, 4 , 0x1f, &hh_test_read_last);

	//	write utp ext regOxa003: 0xf0    设置延时
	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xa003);
	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0x3f);

	// LED 配置
	//	write utp ext regOxa00D: 0x2600    设置 LED1 发包闪烁
	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xA00D);
	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0x2600);
	//	write utp ext regOxa00E: 0xde00    设置 LED2 link常亮
	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xA00E);
	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, 0xDE00);

	//read   ext regOxa003: 0xf0  --test
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xa001);
//	XEmacPs_PhyRead(xemacp, 4 , 0x1f, &hh_test_read_new);
//
//	hh_test_read_new &= 0xFEFF;
//
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1e, 0xa001);
//	XEmacPs_PhyWrite(xemacp, 4 , 0x1f, hh_test_read_new);
//


//
//	XEmacPs_PhyRead(xemacp, 4 , 0x01, &hh_test_read_0x01);
//	XEmacPs_PhyRead(xemacp, 4 , 0x01, &hh_test_read_0x01);
//	XEmacPs_PhyRead(xemacp, 4 , 0x11, &hh_test_read_0x11);

}

extern Uint8 local_lip[4];
void PS_LWIP_Init(void)
{
	struct netif *netif;
	ip_addr_t ipaddr, netmask, gw;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;

	// 防止 mac 地址冲突
	mac_ethernet_address[0] = mgmd_stSCIRx.ip.addr1;
	mac_ethernet_address[1] = mgmd_stSCIRx.ip.addr2;
	mac_ethernet_address[2] = mgmd_stSCIRx.ip.addr3;
	mac_ethernet_address[3] = mgmd_stSCIRx.ip.addr4;

	// 设置定时器中断，已经单独初始化过了，此处不需要
//	init_platform();

	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr,  mgmd_stSCIRx.ip.addr1, mgmd_stSCIRx.ip.addr2, mgmd_stSCIRx.ip.addr3, mgmd_stSCIRx.ip.addr4);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   1,  1);

	/* initialize lwIP */
	lwip_init();


	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		return;
	}

	ReadPhyReg();

	netif_set_default(netif);

	/* now enable interrupts */
//	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(netif);

	etharp_gratuitous(netif);	//主动广播设备的 IP 和 MAC 地址给局域网的所有设备，从而加快 ARP 表的更新速度
//	assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));

	/* start the application*/
	start_application();
}

void eth_test_link(struct netif *netif);

void RunTcpServerMain(void)
{
	struct netif *netif = &server_netif;
	static Uint32 nowtime_250 = 0;
	static Uint32 nowtime_500 = 0;

	if(task_run_cnt - nowtime_250 > Tms_CNT(250))
	{
		nowtime_250 = task_run_cnt;
		//250ms执行一次
		tcp_fasttmr();
	}
	if(task_run_cnt - nowtime_500 > Tms_CNT(500))
	{
		nowtime_500 = task_run_cnt;
		//500ms执行一次
		tcp_slowtmr();

		eth_link_detect(netif);
	}

	xemacif_input(netif);
}



