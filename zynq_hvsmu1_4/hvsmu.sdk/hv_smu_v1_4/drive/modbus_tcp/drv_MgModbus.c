#include "COMMON.h"

/**************************************************************************************************
CONSTANT DESCRIPTION                                                               
**************************************************************************************************/

#define SLAVEID		1
void InitModbusData();


void TaskModbusTCPInit(void)
{
   ModbusDataRegister();
   InitModbusData();
}

Uint16 ModbusTCPHandle(Uint8* recv_buf, Uint16 recv_len, Uint8* send_buf)
{
	Uint16 send_len = 0;

    if (JudgeModbusTCP(recv_buf, recv_len) == 1)
    {
    	//接收数据并回复
    	send_len = ModbusTCPParse(recv_buf, send_buf);
    }

    return send_len;
}

//判断是否符合modbusTCP协议
//符合条件返回1，否则返回0
Uint16 JudgeModbusTCP(Uint8* buf, Uint16 len)
{
    Uint16 tmp_len = 0;
    //addr
    if(buf[6] != SLAVEID)
    {
        return 0;
    }

    //fun code
    if((buf[7] != 0x03) && (buf[7] != 0x06) && (buf[7] != 0x10))
    {
        return 0;
    }

    //lenth
    tmp_len = ((Uint16)buf[4] << 8) | buf[5];
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



static Uint16 ReadRegisters(Uint8* pTxdata, Uint16 addr, Uint16 regnum, Uint16* buffer[], Uint16 startaddr, Uint16 endaddr, Uint16* sendnum)
{
    Uint16 i = 0;
    Uint16 addr_offset = 0;
    Uint16 value = 0;
    Uint16 rst = 0;

    if ((addr >= startaddr) && (addr <= endaddr) && ((addr + regnum - 1) <= endaddr))
    {
        for (i = 0; i < regnum; i++)
        {
            addr_offset = addr - startaddr + i;
            value = *buffer[addr_offset];
            pTxdata[(*sendnum)++] = (value >> 8) & 0x00FF;
            pTxdata[(*sendnum)++] = value & 0x00FF;
        }
        rst = 1;
    }

    return rst;
}


static Uint16 WriteSingleRegister(Uint16 addr, Uint16 value, Uint16* buffer[], Uint16 startaddr, Uint16 endaddr)
{
    Uint16 addr_offset;
    Uint16 rst = 0;
    if ((addr >= startaddr) && (addr <= endaddr))
    {
        addr_offset = addr - startaddr;
        *buffer[addr_offset] = value;
        rst = 1;
    }

    return rst;
}


static Uint16 WriteMultipleRegisters(Uint16 addr, Uint16 regnum, Uint8* pRxdata, Uint16* buffer[], Uint16 startaddr, Uint16 endaddr,  Uint16 header_offset)
{
    Uint16 i;
    Uint16 addr_offset;
    Uint16 value;
    Uint16 value_H;
    Uint16 value_L;
    Uint16 rst = 0;

    if ((addr >= startaddr) && (addr <= endaddr) && ((addr + regnum - 1) <= endaddr))
    {
        addr_offset = addr - startaddr;
        for (i = 0; i < regnum; i++)
        {
            value_H = pRxdata[header_offset + i * 2];
            value_L = pRxdata[header_offset + i * 2 + 1];
            value = (value_H << 8) | value_L;
            *buffer[addr_offset + i] = value;
        }
        rst = 1;
    }

    return rst;
}


static void AddrErrHandle(Uint8* pTxdata, Uint16 funcode, Uint16* sendnum)
{
    *sendnum = 6;
    pTxdata[(*sendnum)++] = SLAVEID;
    pTxdata[(*sendnum)++] = funcode + 0x80;
    pTxdata[(*sendnum)++] = 0x02;
}

static Uint16 MsgHandle(Uint8* pRxdata, Uint8* pTxdata)
{
   Uint16 u16RegValue = ((Uint16)pRxdata[10] << 8) | pRxdata[11];
   Uint16 funcode = pRxdata[7];
   Uint16 startaddr = ((Uint16)pRxdata[8] << 8) | pRxdata[9];
   Uint16 regnum = ((Uint16)pRxdata[10] << 8) | pRxdata[11];
   Uint16 sendnum = 6;
   Uint16 rmulti_rst = 0;
   Uint16 wsigle_rst = 0;
   Uint16 wmulti_rst = 0;
   Uint16 i = 0;

   switch(funcode)
   {
        case READ_MULT:
            pTxdata[sendnum++] = SLAVEID;
            pTxdata[sendnum++] = funcode;
            pTxdata[sendnum++] = regnum*2;

            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_r1_buf, MD_R1_START_ADDR, MD_R1_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_r2_buf, MD_R2_START_ADDR, MD_R2_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_rw1_buf, MD_RW1_START_ADDR, MD_RW1_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_rw2_buf, MD_RW2_START_ADDR, MD_RW2_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_rw3_buf, MD_RW3_START_ADDR, MD_RW3_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_rw4_buf, MD_RW4_START_ADDR, MD_RW4_END_ADDR, &sendnum);
            if(rmulti_rst == 0)     rmulti_rst = ReadRegisters(pTxdata, startaddr, regnum, md_w1_buf, MD_W1_START_ADDR, MD_W1_END_ADDR, &sendnum);
            break;

        case WRITE_SINGLE:
            //organize tx data
            for(i = 6; i <= 11; i++)
            {
                pTxdata[sendnum++] = pRxdata[i];
            }

            if(wsigle_rst == 0)     wsigle_rst = WriteSingleRegister(startaddr, u16RegValue, md_rw1_buf, MD_RW1_START_ADDR, MD_RW1_END_ADDR);
            if(wsigle_rst == 0)     wsigle_rst = WriteSingleRegister(startaddr, u16RegValue, md_rw2_buf, MD_RW2_START_ADDR, MD_RW2_END_ADDR);
            if(wsigle_rst == 0)     wsigle_rst = WriteSingleRegister(startaddr, u16RegValue, md_rw3_buf, MD_RW3_START_ADDR, MD_RW3_END_ADDR);
            if(wsigle_rst == 0)     wsigle_rst = WriteSingleRegister(startaddr, u16RegValue, md_rw4_buf, MD_RW4_START_ADDR, MD_RW4_END_ADDR);
            if(wsigle_rst == 0)     wsigle_rst = WriteSingleRegister(startaddr, u16RegValue, md_w1_buf, MD_W1_START_ADDR, MD_W1_END_ADDR);
            break;

        case WRITE_MULT:
            for(i = 6; i <= 11; i++)
            {
                pTxdata[sendnum++] = pRxdata[i];
            }

            if(wmulti_rst == 0)     wmulti_rst = WriteMultipleRegisters(startaddr, regnum, pRxdata, md_rw1_buf, MD_RW1_START_ADDR, MD_RW1_END_ADDR, MD_DATA_OFFSET);
            if(wmulti_rst == 0)     wmulti_rst = WriteMultipleRegisters(startaddr, regnum, pRxdata, md_rw2_buf, MD_RW2_START_ADDR, MD_RW2_END_ADDR, MD_DATA_OFFSET);
            if(wmulti_rst == 0)     wmulti_rst = WriteMultipleRegisters(startaddr, regnum, pRxdata, md_rw3_buf, MD_RW3_START_ADDR, MD_RW3_END_ADDR, MD_DATA_OFFSET);
            if(wmulti_rst == 0)     wmulti_rst = WriteMultipleRegisters(startaddr, regnum, pRxdata, md_rw4_buf, MD_RW4_START_ADDR, MD_RW4_END_ADDR, MD_DATA_OFFSET);
            if(wmulti_rst == 0)     wmulti_rst = WriteMultipleRegisters(startaddr, regnum, pRxdata, md_w1_buf, MD_W1_START_ADDR, MD_W1_END_ADDR, MD_DATA_OFFSET);
           break;
        default:
           break;
   }

   if(rmulti_rst == 0 && wsigle_rst == 0 && wmulti_rst == 0)
       AddrErrHandle(pTxdata, funcode, &sendnum);

   return sendnum;
}

/**************************************************************************************************
FUNCTION: mgmd_vPollTb1
DESCRIPTION: mgcc modbus poll
**************************************************************************************************/
Uint16 ModbusTCPParse(Uint8* prxdata, Uint8* ptxdata)
{
   int i;
   Uint16 sendnum = 0;

   //reply to received frame header(the first four byte)
   for(i = 0; i < 4; i++)
   {
       ptxdata[i] = prxdata[i];
   }

   //for compatibility
   mgmd_stSCIRx.reserve = 0;

   sendnum = MsgHandle(prxdata, ptxdata);

   //lenth
   ptxdata[4] = ((sendnum-6) >> 8) & 0x00FF;
   ptxdata[5] = ((sendnum-6) & 0x00FF);

   return sendnum;
}


/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
