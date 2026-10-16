#include "drv_McBSP.h"
#include <string.h>
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File



Uint16 McBSPb_Xmit16(Uint16 senddata)
{
    Uint16 readdata = 0;

    McbspbRegs.DXR1.all = senddata;

    while( McbspbRegs.SPCR1.bit.RRDY == 0 ) {}   // Master waits until RX data is ready
    readdata = McbspbRegs.DRR1.all;              // Then read DRR1

    return readdata;
}


Uint32 ReadFPGA32(Uint16 addr)
{
    Uint16 dataH = 0, dataL = 0;
    Uint32 data = 0;

    McBSPb_Xmit16((1<<8) | (addr & 0xFF));
    dataH = McBSPb_Xmit16(0);
    dataL = McBSPb_Xmit16(0);

    data = ((Uint32)dataH << 16) | dataL;

    return data;
}


void WriteFPGA32(Uint16 addr, Uint32 data)
{
    McBSPb_Xmit16((0<<8) | (addr & 0xFF));
    McBSPb_Xmit16((data >> 16) & 0xFFFF);
    McBSPb_Xmit16(data & 0xFFFF);
}
