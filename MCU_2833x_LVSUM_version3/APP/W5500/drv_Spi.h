#ifndef _DRVSPI_H_
#define _DRVSPI_H_


#define SOCKET_NUM_USE  3

/* Private function prototypes -----------------------------------------------*/
typedef struct
{
    Uint32 recv_all_cnt;   //总的接收的帧计数
    Uint32 recv_frame_correct_cnt; // 正确计数
    Uint32 recv_frame_err_cnt;   //帧格式错误计数
    Uint32 send_frame_cnt;   //发送帧计数
    Uint32 overtime_est_cnt;  //建立连接超时计数
}MODBUS_TCP_CNT;
extern MODBUS_TCP_CNT    modbus_tcp_cnt;

extern Uint16 W5500_trans_flag;
extern Uint16 W5500_trans_len;
extern Uint8 W5500_recv_buff[];       // 网口接收缓存   ，长度与串口modbus的缓存一致
extern Uint8 W5500_trans_buff[];
extern Uint16 W5500_established_flag;


extern Uint8 SPI2_SendByte(Uint8 byte);
extern void WIZ_CS(Uint8 val);

extern void Reset_W5500(void);
extern void do_tcp_server(Uint8 s, Uint16 index);
extern void Init_W5500();
extern void CheckW5500Status(void);
#endif

