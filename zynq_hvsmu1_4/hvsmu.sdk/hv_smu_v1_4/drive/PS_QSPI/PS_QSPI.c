/*
 * PS_QSPI.c
 *
 *  Created on: 2025年8月20日
 *      Author: m
 */


#include "COMMON.h"


#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

//发送到FLASH器件的指令
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD			0x02
#define READ_CMD			0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID				0x9F

//FLASH BUFFER中各数据的偏移量
#define COMMAND_OFFSET		0  // FLASH instruction */
#define ADDRESS_1_OFFSET	1  // MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2  // Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3  // LSB byte of address to read or write */
#define DATA_OFFSET			4  // Start of Data for Read/Write */
#define DUMMY_OFFSET		4  // Dummy byte offset for fast, dual and quad reads
#define DUMMY_SIZE			1  // Number of dummy bytes for fast, dual and quad reads
#define RD_ID_SIZE			4  // Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1  // Bulk Erase command size */
#define SEC_ERASE_SIZE		4  // Sector Erase command + Sector address */
#define OVERHEAD_SIZE		4  // control information: command and address

#define SECTOR_SIZE		0x10000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		256
#define PAGE_MASK 		(PAGE_SIZE - 1)


void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);
int FlashReadID(void);
void FlashQuadEnable(XQspiPs *QspiPtr);

XQspiPs QspiInstance;
Uint8 ReadBuffer[PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE];
Uint8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];


void PS_QSPI_Init(void)
{
    XQspiPs_Config *QspiConfig;

    //初始化QSPI驱动
    QspiConfig = XQspiPs_LookupConfig(QSPI_DEVICE_ID);
    XQspiPs_CfgInitialize(&QspiInstance, QspiConfig, QspiConfig->BaseAddress);

    //设置自动启动和手动片选模式
    XQspiPs_SetOptions(&QspiInstance,
    XQSPIPS_FORCE_SSELECT_OPTION | XQSPIPS_HOLD_B_DRIVE_OPTION);
    //设置QSPI时钟的分频系数
    XQspiPs_SetClkPrescaler(&QspiInstance, XQSPIPS_CLK_PRESCALE_8);
    //片选信号置为有效
    XQspiPs_SetSlaveSelect(&QspiInstance);
    //使能FLASH Quad模式
    FlashQuadEnable(&QspiInstance);
}

//#define SECTOR_SIZE		0x10000
//#define NUM_SECTORS		0x100
void UserFlashErase(Uint16 sector_start, Uint16 num)
{
	Uint32 addr;

	addr = sector_start * SECTOR_SIZE;
	if(num > NUM_SECTORS)
		num = NUM_SECTORS;
	else if(num == 0)
		return;

    //擦除FLASH
    FlashErase(&QspiInstance, addr, SECTOR_SIZE * num);
}

// 该接口地址只能是 PAGE_SIZE 的整数倍！
void UserFlashWrite(const Uint8 *pbuf, Uint32 addr, Uint32 num)
{
	Uint32 writed_len = 0;
	Uint32 write_addr = 0;

	write_addr = addr;
	writed_len = 0;
	while(writed_len + PAGE_SIZE < num)
	{
		memcpy(&WriteBuffer[DATA_OFFSET], &pbuf[writed_len], PAGE_SIZE);
		FlashWrite(&QspiInstance, write_addr, PAGE_SIZE, WRITE_CMD);
		write_addr += PAGE_SIZE;
		writed_len += PAGE_SIZE;
	}
	// 最后一次
	memcpy(&WriteBuffer[DATA_OFFSET], &pbuf[writed_len], num - writed_len);
	FlashWrite(&QspiInstance, write_addr, num - writed_len, WRITE_CMD);
}

// 该接口地址只能是 PAGE_SIZE 的整数倍！
void UserFlashRead(Uint8 *pbuf, Uint32 addr, Uint32 num)
{
	Uint32 readed_len = 0;
	Uint32 read_addr = 0;

	read_addr = addr;
	readed_len = 0;
	while(readed_len + PAGE_SIZE < num)
	{
	    //使用QUAD模式从FLASH中读出数据并进行校验
		FlashRead(&QspiInstance, read_addr, PAGE_SIZE, QUAD_READ_CMD);
		memcpy(&pbuf[readed_len], &ReadBuffer[DATA_OFFSET + DUMMY_SIZE], PAGE_SIZE);
		read_addr += PAGE_SIZE;
		readed_len += PAGE_SIZE;
	}
	// 最后一次
	FlashRead(&QspiInstance, read_addr, num - readed_len, QUAD_READ_CMD);
	memcpy(&pbuf[readed_len], &ReadBuffer[DATA_OFFSET + DUMMY_SIZE], num - readed_len);
}




Uint16 hh_test_flash = 0;
#define TEST_SIZE	600
Uint8 test_write_data[TEST_SIZE];
Uint8 test_read_data[TEST_SIZE];
Uint8 test_write = 1;
void TestMainFlash(void)
{
    Uint16 i = 0;

	if(hh_test_flash == 1)
	{
		hh_test_flash = 0;
		UserFlashErase(0x40, 1);
	}
	else if(hh_test_flash == 2)
	{
		hh_test_flash = 0;
		UserFlashWrite(test_write_data, 0x400000, TEST_SIZE);
	}
	else if(hh_test_flash == 3)
	{
		hh_test_flash = 0;

		UserFlashRead(test_read_data, 0x400000, TEST_SIZE);
	}
	else if(hh_test_flash == 4)
	{
		hh_test_flash = 0;
		for(i = 0; i < sizeof(test_write_data); i++)
		{
			test_write_data[i] = test_write + i;
		}
	}
}


// 从 0 地址开始写入，写入完再全部回读出来做校验，校验成功返回0
Uint16 QspiUpdate(u32 total_bytes, const u8 *flash_data)
{
    Uint8 read_buf[PAGE_SIZE];
    Uint32 readed_len = 0;
    Uint32 read_addr = 0;
    Uint16 i;
    Uint16 total_sector = (total_bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;

    //擦除FLASH
    UserFlashErase(0, total_sector);
    //向FLASH中写入数据
    UserFlashWrite(flash_data, 0, total_bytes);
    //从FLASH中读出数据并进行校验
    read_addr = 0;
    readed_len = 0;
    while (readed_len < total_bytes)
    {
    	UserFlashRead(read_buf, read_addr, PAGE_SIZE);
        if ((readed_len + PAGE_SIZE) <= total_bytes)
        {
            //对比写入FLASH与从FLASH中读出的数据
            for (i = 0; i < PAGE_SIZE; i++)
                if (read_buf[i] != flash_data[readed_len + i])
                    return 1;
        }
        else
        {
            for (i = 0; i < total_bytes - readed_len; i++)
                if (read_buf[i] != flash_data[readed_len + i])
                	return 1;
        }
        readed_len += PAGE_SIZE;
        read_addr += PAGE_SIZE;
    }

    return 0;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial FLASH connected to the QSPI interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 * @param	Address contains the address to write data to in the FLASH.
 * @param	ByteCount contains the number of bytes to write.
 * @param	Command is the command used to write data to the flash. QSPI
 *		device supports only Page Program command to write data to the
 *		flash.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
    u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
    u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 }; /* must send 2 bytes */
    u8 FlashStatus[2];

    /*
     * Send the write enable command to the FLASH so that it can be
     * written to, this needs to be sent as a seperate transfer before
     * the write
     */
    XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
            sizeof(WriteEnableCmd));

    /*
     * Setup the write command with the specified address and data for the
     * FLASH
     */
    WriteBuffer[COMMAND_OFFSET] = Command;
    WriteBuffer[ADDRESS_1_OFFSET] = (u8) ((Address & 0xFF0000) >> 16);
    WriteBuffer[ADDRESS_2_OFFSET] = (u8) ((Address & 0xFF00) >> 8);
    WriteBuffer[ADDRESS_3_OFFSET] = (u8) (Address & 0xFF);

    /*
     * Send the write command, address, and data to the FLASH to be
     * written, no receive buffer is specified since there is nothing to
     * receive
     */
    XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
            ByteCount + OVERHEAD_SIZE);

    /*
     * Wait for the write command to the FLASH to be completed, it takes
     * some time for the data to be written
     */
    while (1) {
        /*
         * Poll the status register of the FLASH to determine when it
         * completes, by sending a read status command and receiving the
         * status byte
         */
        XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
                sizeof(ReadStatusCmd));

        /*
         * If the status indicates the write is done, then stop waiting,
         * if a value of 0xFF in the status byte is read from the
         * device and this loop never exits, the device slave select is
         * possibly incorrect such that the device status is not being
         * read
         */
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
    }
}

/*****************************************************************************/
/**
 *
 * This function reads from the  serial FLASH connected to the
 * QSPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 * @param	Address contains the address to read data from in the FLASH.
 * @param	ByteCount contains the number of bytes to read.
 * @param	Command is the command used to read data from the flash. QSPI
 *		device supports one of the Read, Fast Read, Dual Read and Fast
 *		Read commands to read data from the flash.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
    /*
     * Setup the write command with the specified address and data for the
     * FLASH
     */
    WriteBuffer[COMMAND_OFFSET] = Command;
    WriteBuffer[ADDRESS_1_OFFSET] = (u8) ((Address & 0xFF0000) >> 16);
    WriteBuffer[ADDRESS_2_OFFSET] = (u8) ((Address & 0xFF00) >> 8);
    WriteBuffer[ADDRESS_3_OFFSET] = (u8) (Address & 0xFF);

    if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD)
            || (Command == QUAD_READ_CMD)) {
        ByteCount += DUMMY_SIZE;
    }
    /*
     * Send the read command to the FLASH to read the specified number
     * of bytes from the FLASH, send the read command and address and
     * receive the specified number of bytes of data in the data buffer
     */
    XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer,
            ByteCount + OVERHEAD_SIZE);
}

/*****************************************************************************/
/**
 *
 * This function erases the sectors in the  serial FLASH connected to the
 * QSPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *		be erased.
 * @param	ByteCount contains the total size to be erased.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
    u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
    u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 }; /* must send 2 bytes */
    u8 FlashStatus[2];
    int Sector;
    int total_sector;
    /*
     * If erase size is same as the total size of the flash, use bulk erase
     * command
     */
    if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
        /*
         * Send the write enable command to the FLASH so that it can be
         * written to, this needs to be sent as a seperate transfer
         * before the erase
         */
        XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
                sizeof(WriteEnableCmd));

        /* Setup the bulk erase command*/
        WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;

        /*
         * Send the bulk erase command; no receive buffer is specified
         * since there is nothing to receive
         */
        XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
        BULK_ERASE_SIZE);

        /* Wait for the erase command to the FLASH to be completed*/
        while (1) {
            /*
             * Poll the status register of the device to determine
             * when it completes, by sending a read status command
             * and receiving the status byte
             */
            XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
                    sizeof(ReadStatusCmd));

            /*
             * If the status indicates the write is done, then stop
             * waiting; if a value of 0xFF in the status byte is
             * read from the device and this loop never exits, the
             * device slave select is possibly incorrect such that
             * the device status is not being read
             */
            if ((FlashStatus[1] & 0x01) == 0) {
                break;
            }
        }

        return;
    }

    /*
     * If the erase size is less than the total size of the flash, use
     * sector erase command
     */
    total_sector = (ByteCount / SECTOR_SIZE) + 1;
    for (Sector = 0; Sector < total_sector; Sector++)
    {

        /*
         * Send the write enable command to the SEEPOM so that it can be
         * written to, this needs to be sent as a seperate transfer
         * before the write
         */
        XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
                sizeof(WriteEnableCmd));

        /*
         * Setup the write command with the specified address and data
         * for the FLASH
         */
        WriteBuffer[COMMAND_OFFSET] = SEC_ERASE_CMD;
        WriteBuffer[ADDRESS_1_OFFSET] = (u8) (Address >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (u8) (Address >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (u8) (Address & 0xFF);

        /*
         * Send the sector erase command and address; no receive buffer
         * is specified since there is nothing to receive
         */
        XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
        SEC_ERASE_SIZE);

        /*
         * Wait for the sector erse command to the
         * FLASH to be completed
         */
        while (1) {
            /*
             * Poll the status register of the device to determine
             * when it completes, by sending a read status command
             * and receiving the status byte
             */
            XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
                    sizeof(ReadStatusCmd));

            /*
             * If the status indicates the write is done, then stop
             * waiting, if a value of 0xFF in the status byte is
             * read from the device and this loop never exits, the
             * device slave select is possibly incorrect such that
             * the device status is not being read
             */
            if ((FlashStatus[1] & 0x01) == 0) {
                break;
            }
        }

        Address += SECTOR_SIZE;
    }
}

/*****************************************************************************/
/**
 *
 * This function reads serial FLASH ID connected to the SPI interface.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 ******************************************************************************/
int FlashReadID(void)
{
    int Status;

    /* Read ID in Auto mode.*/
    WriteBuffer[COMMAND_OFFSET] = READ_ID;
    WriteBuffer[ADDRESS_1_OFFSET] = 0x23; /* 3 dummy bytes */
    WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
    WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

    Status = XQspiPs_PolledTransfer(&QspiInstance, WriteBuffer, ReadBuffer,
    RD_ID_SIZE);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/

/**
 *
 * This function enables quad mode in the serial flash connected to the
 * SPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashQuadEnable(XQspiPs *QspiPtr)
{
    u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
    u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };
    u8 QuadEnableCmd[] = { WRITE_STATUS_CMD, 0 };
    u8 FlashStatus[2];

    if (ReadBuffer[1] == 0x9D) {

        XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
                sizeof(ReadStatusCmd));

        QuadEnableCmd[1] = FlashStatus[1] | 1 << 6;

        XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
                sizeof(WriteEnableCmd));

        XQspiPs_PolledTransfer(QspiPtr, QuadEnableCmd, NULL,
                sizeof(QuadEnableCmd));
    }
}

