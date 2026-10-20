#include <task_uph_common.h>
#include "COMMON.h"

Uint32 RecvParaAndSend(Uint8* precvbuf, Uint8* ptxbuf);

ST_SELF_PROTOCOL  st_selfprotocol;
ST_COM_CRTLCARD     st_com_ctrl;
ST_FROM_PC     st_frompc;
ST_TO_PC     st_topc;
ST_UPH    st_uph;

//1. 接收上位机指令
//2. 与主控板交互

void Init_taskUPH(void)
{
    memset(&st_selfprotocol, 0, sizeof(st_selfprotocol));
    memset(&st_com_ctrl, 0, sizeof(st_com_ctrl));
    memset(&st_frompc, 0, sizeof(st_frompc));
    memset(&st_topc, 0, sizeof(st_topc));
    memset(&st_uph, 0, sizeof(st_uph));

    st_uph.com.total_para_num = sizeof(st_frompc) / sizeof(float32);
    st_uph.com.num_per_group = sizeof(st_frompc.para_group[0]) / sizeof(float32);

    InitPara();
}




//1. 上位机交互、多发模式
void MultiModeFSM(void)
{
    ST_FROM_PC* pPc = &st_frompc;
    ST_UPH* p = &st_uph;
    ST_COM_CRTLCARD*  pCtrl = &st_com_ctrl;
    Uint16 i = 0, j = 0;

    //每次上位机下发参数都强制复位状态机
    if(p->recv_ok == 1)
    {
        p->recv_ok = 0;
        p->state = PROCESS_PARA;
    }

    //只有在刚开始计算参数阶段，允许接收start指令，别的时候都把start清零，防止误接收
    if(p->state != PROCESS_PARA && p->state != WAIT_START && p->state != IDEL)
        pCtrl->start = 0;

    //在wait start时，先接收到trigger就把trigger清零
    if(p->state == WAIT_START && pCtrl->start == 0)
        pCtrl->trigger = 0;

    //接收到stop以后将状态复位到wait start状态，并回复主板卡stop ok
    if(pCtrl->stop == 1)
    {
        pCtrl->stop = 0;
        SendRstOnce(RST_STOP);
        p->state = WAIT_START;
    }

    switch(p->state)
    {
    case IDEL:
        //test--用初始化自定义的参数
//        p->state = PROCESS_PARA;
        break;

    case PROCESS_PARA:
        //计算总动作数量、下发参数合规性判断等
        p->trigger_num.total = 0;
        for(i = 0; i < pPc->group_num; i++)
        {
            p->trigger_num.group_per[i] = 0;
            for(j = 0; j < TRIGGER_NUM; j++)
            {
                if(pPc->para_group[i].trigger[j] != 0)
                {
                    p->trigger_num.total++;
                    p->trigger_num.group_per[i]++;
                }
            }
        }
        // 将group_num返回给上位机
        st_topc.info.bit.group_num = pPc->group_num;
        // 状态跳转
        p->state = WAIT_START;
        break;

    //状态机正常应该就是在这个状态，等待主控板start命令
    case WAIT_START:
        if(pCtrl->start == 1)
        {
            pCtrl->start = 0;
            p->trigger_num.remain = p->trigger_num.total;
            p->group_index = 0;
            p->trigger_index = 0;
            pCtrl->trigger = 0;
            //接收主控板start命令后，给主控板返回工序数量，123代表结果无效，仅发工序数量
            SendRstOnce(RST_START);
            p->state = SET_PARA;
        }
        break;

    case SET_PARA:
        SetPara(p->group_index);
        p->overtime_ms = GetOverTimeMs(p->group_index);
        p->state = TRIGGER;
        break;

    case TRIGGER:
        if(pCtrl->trigger == 1)
        {
            pCtrl->trigger = 0;
            TriggerFunc((Uint16)pPc->para_group[p->group_index].trigger[p->trigger_index]);
            p->state = RESULT;
        }
        break;

    case RESULT:
        //本次触发结果是否ok，并记录反馈值
        p->result_cnt++;
        p->result = CheckResult((Uint16)pPc->para_group[p->group_index].trigger[p->trigger_index], p->group_index);
        if((p->result != RST_OVERTIME) || (p->result_cnt > Tms_CNT(p->overtime_ms)))
        {
            p->result_cnt = 0;
            p->state = SEND_RST;
        }
        break;

    case SEND_RST:
        //先自减再回复主控板
        p->trigger_index++;
        p->trigger_num.remain--;
        SendRstOnce(p->result);
        if(p->trigger_index == p->trigger_num.group_per[p->group_index])
        {
            p->trigger_index = 0;
            p->group_index++;
            if(p->group_index == pPc->group_num)
            {
                p->group_index = 0;
                p->state = WAIT_START;   //全部工序打完，回到等待主控板start状态
            }
            else
            {
                p->state = SET_PARA;
            }
        }
        else
        {
            p->state = TRIGGER;
        }
        break;

//    case IDEL:
//        p->state = IDEL;
//        break;

    default: p->state = IDEL;
    }
}

//将上位机下发的datanum个数的4字节数据转化为float数据，并且赋值给本地的一片内存空间
void U8ToFloat(Uint8* pcbuf, Uint16 datanum, Uint32* userbuf)
{
    CONV_U8*  p_u8 = (CONV_U8*)pcbuf;
    CONV_F32 tmp_f32;
    Uint16 i = 0;

    for(i = 0; i < datanum; i++)
    {
        tmp_f32.u8.HH = (p_u8 + i)->u8_1;
        tmp_f32.u8.HL = (p_u8 + i)->u8_2;
        tmp_f32.u8.LH = (p_u8 + i)->u8_3;
        tmp_f32.u8.LL = (p_u8 + i)->u8_4;

        *(userbuf + i) = tmp_f32.u32;
    }
}

//将本地的datanum个数的float数据，转化为发给上位机的4字节数据
void FloatToU8(Uint8* pcbuf, Uint16 datanum, Uint32* userbuf)
{
    CONV_U8*  p_u8 = (CONV_U8*)pcbuf;
    CONV_F32 tmp_f32;
    Uint16 i = 0;

    for(i = 0; i < datanum; i++)
    {
        tmp_f32.u32 = *(userbuf + i);

        (p_u8 + i)->u8_1 = tmp_f32.u8.HH;
        (p_u8 + i)->u8_2 = tmp_f32.u8.HL;
        (p_u8 + i)->u8_3 = tmp_f32.u8.LH;
        (p_u8 + i)->u8_4 = tmp_f32.u8.LL;
    }
}

//判断是否符合自定义协议
//0 功能码   1 2起始地址   3 4读、写个数    每个数据都是float类型固定
Uint32 HighUphHandle(Uint8* recv_buf, Uint16 recv_len, Uint8 *send_buf)
{
    ST_SELF_PROTOCOL* pself = &st_selfprotocol;
    Uint32 send_len = 0;

    pself->funcode = recv_buf[0];
    pself->startaddr = recv_buf[2] + ((Uint16)recv_buf[1] << 8);
    pself->datanum = recv_buf[4] + ((Uint16)recv_buf[3] << 8);

    //addr
    if(pself->funcode != FUNCOODE_W && pself->funcode != FUNCOODE_R)
        return 0;

    if(pself->funcode == FUNCOODE_W && pself->datanum != ((recv_len - SELF_PROTOCOL_HEAD) >> 2))
    	return 0;

    if(pself->funcode == FUNCOODE_R && recv_len != SELF_PROTOCOL_HEAD)
    	return 0;

    if(pself->datanum + pself->startaddr > st_uph.com.total_para_num)
    	return 0;

    send_len = RecvParaAndSend(recv_buf, send_buf);
    return send_len;
}

//start 是指第几个float
//num 是指本次下发了几个float
//对于上位机来说float是4字节的
Uint32 RecvParaAndSend(Uint8* precvbuf, Uint8* ptxbuf)
{
    ST_SELF_PROTOCOL* pself = &st_selfprotocol;
    ST_UPH*     p = &st_uph;
    ST_FROM_PC*   pfrom = &st_frompc;
    ST_TO_PC*     pto = &st_topc;
    Uint16 i = 0;

    for(i = 0; i < SELF_PROTOCOL_HEAD; i++)
    {
        *(ptxbuf + i) = *(precvbuf + i);
    }

    p->com.recv_cnt++;

    //接收并回复
    if(pself->funcode == FUNCOODE_W)
    {
        U8ToFloat(precvbuf + SELF_PROTOCOL_HEAD, pself->datanum, (Uint32*)pfrom + pself->startaddr);

        if(pself->startaddr == 0)
            p->com.recv_num_tmp = pself->datanum;
        else
            p->com.recv_num_tmp += pself->datanum;

        //把功能码、起始地址、数量回给上位机
        pself->send_len = SELF_PROTOCOL_HEAD;

        //计算参数是否全部接收ok
        p->com.recv_ok_num = p->com.num_per_group * pfrom->group_num + GROUP_ADDR;
        if(p->com.recv_num_tmp == p->com.recv_ok_num)
        {
            p->com.recv_num_tmp = 0;
            p->recv_ok = 1;
            p->com.para_ok_cnt++;
        }
    }
    else if(pself->funcode == FUNCOODE_R)
    {
        FloatToU8(ptxbuf + SELF_PROTOCOL_HEAD, pself->datanum, (Uint32*)pto + pself->startaddr);
        pself->send_len = SELF_PROTOCOL_HEAD + (pself->datanum << 2);
    }

    return (Uint32)pself->send_len;
}


Uint16 ReadData(Uint16 addr)
{
    ST_UPH*     pUser = &st_uph;
//    ST_COM_CRTLCARD*  pCtrl = &st_com_ctrl;

    Uint16 data = 0;
    switch(addr)
    {
    case R_REMAIN_NUM:      data = pUser->trigger_num.remain;
        break;
//    case R_DEVICE_NO:       data = DEVICE_LV;
//        break;
    case R_STATE:           data = pUser->state;
        break;
    default:                data = 123;
    }

    return data;
}


void WriteData(Uint16 addr, Uint16 data)
{
//    ST_UPH*     pUser = &st_uph;
    ST_COM_CRTLCARD*  pCtrl = &st_com_ctrl;

    switch(addr)
    {
    case W_START:           pCtrl->start = data;
        break;
    case W_TRIGGER:         pCtrl->trigger = data;
        break;
    case W_STOP:            pCtrl->stop = data;
        break;
    default:
        break;
    }
}


void WriteMeterStart()
{
    WriteData(W_START, 1);
}
void WriteMeterStop()
{
    WriteData(W_STOP, 1);
}
void WriteMeterTrig()
{
    WriteData(W_TRIGGER, 1);
}
Uint16 ReadMeterStatus()
{
    return ReadData(R_STATE);
}
Uint16 ReadMeterItem()
{
    return ReadData(R_REMAIN_NUM);
}




