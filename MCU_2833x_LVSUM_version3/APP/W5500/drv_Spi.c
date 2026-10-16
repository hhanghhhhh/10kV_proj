////###########################################################################		
// Original Author: S.S.
//
// $TI Release: DSP2833x Header Files V1.01 $
// $Release Date: September 26, 2007 $
//###########################################################################


#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#include "device.h"
#include "socket.h"
#include "w5500.h"
#include "string.h"
#include "drv_Spi.h"
#include "task_wave.h"
#include "drv_GlobalVar.h"
#include "../Comm/drv_MgModbus.h"
// Prototype statements for functions found within this file.
// interrupt void ISRTimer2(void);
MODBUS_TCP_CNT    modbus_tcp_cnt;

Uint8 W5500_recv_buff[1024];       // 网口接收缓存   ，长度与串口modbus的缓存一致
Uint8 W5500_trans_buff[1024];      // 网口发送缓存

Uint16 W5500_trans_flag = 0;
Uint16 W5500_trans_len = 0;
Uint16 W5500_established_flag = 0;

extern Uint16 local_port;
extern Uint16  slINT1s;


void Init_modbusTCP_data()
{
    memset(&modbus_tcp_cnt, 0, sizeof(MODBUS_TCP_CNT));
}

#pragma CODE_SECTION(Init_W5500,"runflashfuncs");
void Init_W5500()
{
    Init_modbusTCP_data();
    Reset_W5500();

    set_default();
    set_network();
}
void WIZ_CS(Uint8 val)
{
    if (val == 0)
        {
            W5500_CSN_L;
        }
        else if (val == 1)
        {
            W5500_CSN_H;
        }
}


Uint8 hh_test_ip[4];
Uint16 reset_w5500_cnt = 0;
extern Uint8 local_lip[];
void CheckW5500Status(void)
{
    static Uint32 now_time_cnt = 0;
    static Uint16 first_flag = 0;

    // 赋初值
    if(first_flag == 0)
    {
        first_flag = 1;
        hh_test_ip[0] = local_lip[0];
        hh_test_ip[1] = local_lip[1];
        hh_test_ip[2] = local_lip[2];
        hh_test_ip[3] = local_lip[3];
    }

    //10s检查一次
    if(task_run_cnt - now_time_cnt > Tms_CNT(10000))
    {
        now_time_cnt = task_run_cnt;
        getSIPR(hh_test_ip);

        if(hh_test_ip[0] == local_lip[0] &&
           hh_test_ip[1] == local_lip[1] &&
           hh_test_ip[2] == local_lip[2] &&
           hh_test_ip[3] == local_lip[3])
        {
            return;
        }

        //DisableDog();
        Init_W5500();
        //EnableWDog();
        reset_w5500_cnt++;
    }
}

Uint8 SPI2_SendByte(Uint8 byte)
{
    Uint8 SPIRXD;
    while(SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1);
    SpiaRegs.SPITXBUF=(byte<<8);
    while(SpiaRegs.SPISTS.bit.INT_FLAG != 1);{}//Wait until data is received
    SPIRXD = SpiaRegs.SPIRXBUF;
    return SPIRXD;

}

#pragma CODE_SECTION(Reset_W5500,"runflashfuncs");
void Reset_W5500(void)//reset
{
    W5500_RESET_L;
    DELAY_US(20000);
    W5500_RESET_H;
    DELAY_US(1600000);  //1.6s
}

//判断是否符合modbusTCP协议
//符合条件返回1，否则返回0
Uint16 modbusTCP_judge(Uint8* buf, Uint16 len)
{
    Uint16 tmp_len = 0;
    //addr
    if(buf[6] != mgmd_u16SlaveID)
    {
        return 0;
    }

    //fun code
    if((buf[7] != 0x03) && (buf[7] != 0x06) && (buf[7] != 0x10))
    {
        return 0;
    }

    //lenth
    tmp_len = ((buf[4] & 0xFF) << 8) | (buf[5] & 0xFF);
    if(buf[7] == 0x10)
    {
        if(len < (tmp_len + 6))
        {
            return 0;
        }
    }
    else
    {
        if(len < 12)
        {
            return 0;
        }
    }

    return 1;
}


// 环形队列
static int16 conn_order[SOCKET_NUM_USE];
static Uint16 head = 0;   // 指向最早的连接
static Uint16 count = 0;  // 当前连接数

// 新连接，保持有一个空的，用于建立新连接
void AddConnect(Uint8 sock_no)
{
    static Uint16 first_flag = 0;
    Uint16 i = 0;

    // 初始化
    if(first_flag == 0)
    {
        first_flag = 1;
        for (i = 0; i < SOCKET_NUM_USE; i++)
        {
            conn_order[i] = -1;
        }
    }

    if (count < SOCKET_NUM_USE - 1)
    {
        Uint16 pos = (head + count) % SOCKET_NUM_USE;
        conn_order[pos] = sock_no;
        count++;
    }
    else
    {
        Uint8 old_sock = conn_order[head];
        close_socket(old_sock);             // 踢掉最早的
        conn_order[head] = -1;              // 把被踢掉的槽显式清空

        head = (head + 1) % SOCKET_NUM_USE; // 队头后移
        // 计算新的“队尾”位置并写入新连接
        Uint16 tail = (head + count - 1) % SOCKET_NUM_USE;
        conn_order[tail] = sock_no;
        // count 不变（仍为 SOCKET_NUM_USE）
    }
}

// 客户退出
void ExitConnect(Uint8 sock_no)
{
    Uint16 i = 0;
    Uint16 j = 0;
    Uint16 last = 0;

    for (i = 0; i < count; i++)
    {
        Uint16 idx = (head + i) % SOCKET_NUM_USE;
        if (conn_order[idx] == sock_no)
        {
            // 后面的元素前移一格
            for (j = i; j < count - 1; j++)
            {
                Uint16 from = (head + j + 1) % SOCKET_NUM_USE;
                Uint16 to   = (head + j) % SOCKET_NUM_USE;
                conn_order[to] = conn_order[from];
            }

            // 把最后的那个位置清空为 -1
            last = (head + count - 1) % SOCKET_NUM_USE;
            conn_order[last] = -1;
            count--;
            return;
        }
    }
}


extern Uint16 mgmd_u16Urt1TxBuf[];
extern Uint16 mgmd_u16Urt1Bytes2Send;
Uint16 sock_tcps_state[3] = {0,0,0}, w5500_linkstate[3] = {0,0,0};  //test，看网口状态用
//W5500作为server端运行，一直处于listen状态，等待client的connect请求
void do_tcp_server(SOCKET s, Uint16 index)
{
    Uint16 len=0;
    int i;
    static Uint16 first_flag_add[SOCKET_NUM_USE] = {0,0,0};
    static Uint16 first_flag_exit[SOCKET_NUM_USE] = {0,0,0};

    sock_tcps_state[index] = getSn_SR(s);
    w5500_linkstate[index] = IINCHIP_READ(PHYCFGR);//(IINCHIP_READ(PHYCFGR)&0x01);
    switch(getSn_SR(s))                                                         /*获取socket的状态*/
    {
        case SOCK_CLOSED:                                                                     /*socket处于关闭状态*/
            socket(s ,Sn_MR_TCP,local_port,Sn_MR_ND);           /*打开socket*/
          break;

        case SOCK_INIT:                                                                       /*socket已初始化状态*/
            listen(s);                                                              /*socket建立监听*/
          break;

        case SOCK_LISTEN:
            first_flag_add[index] = 0;
            if(first_flag_exit[index] == 1)
            {
                ExitConnect(s);
                first_flag_exit[index] = 0;
            }
         break;

        case SOCK_ESTABLISHED:                                                            /*socket处于连接建立状态*/
            if(first_flag_add[index] == 0)
            {
                first_flag_add[index] = 1;
                AddConnect(s);
            }
            first_flag_exit[index] = 1;

            if(getSn_IR(s) & Sn_IR_CON)
            {
                setSn_IR(s, Sn_IR_CON);                                       /*清除接收中断标志位*/
            }
            len=getSn_RX_RSR(s);                                                /*定义len为已接收数据的长度*/
            if(len>0)
            {
                modbus_tcp_cnt.recv_all_cnt++;

                recv(s,W5500_recv_buff,len);                                               /*接收来自Client的数据*/

                //判断数据是否符合modbusTCP协议
                mgmd_u16RxReady = modbusTCP_judge(W5500_recv_buff, len);
                //符合则把接收到的数据给mgmd_u16Urt1RxBuf
                if(mgmd_u16RxReady == 1)
                {
                    for(i = 0; i < len; i++)
                    {
                        mgmd_u16Urt1RxBuf[i] = W5500_recv_buff[i];
                        mgmd_u16Urt1BytesReceived = len;
                    }

                    modbus_tcp_cnt.recv_frame_correct_cnt++;

                    mgmd_vPollTb1();
                    if(W5500_trans_flag == 1)
                    {
                        send(s,(Uint8*)mgmd_u16Urt1TxBuf,mgmd_u16Urt1Bytes2Send);            /*向Client发送数据*/
                        W5500_trans_len = 0;
                        W5500_trans_flag = 0;
                        modbus_tcp_cnt.send_frame_cnt++;
                    }
                }
                else
                {
                    modbus_tcp_cnt.recv_frame_err_cnt++;
                }

                WaveCmdParse(W5500_recv_buff, s, W5500_trans_buff, send);
          }
          WaveSendData(W5500_trans_buff, send);
          break;

        case SOCK_CLOSE_WAIT:                                                               /*socket处于等待关闭状态*/
            close_socket(s);
          break;
    }
}



//===========================================================================
// No more.
//===========================================================================

