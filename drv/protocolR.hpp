/*
 * protocolR.hpp
 *
 *  Created on: Nov 8, 2016
 *      Author: rogal
 */

#ifndef DRV_PROTOCOLR_HPP_
#define DRV_PROTOCOLR_HPP_

#define MAX_DATA_LENGTH		256  /* 252 data bytes + 2 start bytes + 2 stop bytes */
#define USART_COM_MAXFRAME 8+MAX_DATA_LENGTH // START(1)|ADDR(1)|FUNC(1)|WE(2)|CZYT_LEN(1)|CZYT_DATA(256)|CRC(1)|STOP(1)

#define USART_COM_STARTBYTE	  0xA1
#define USART_COM_STOPTBYTE	  0xA2

#define USART_COM_SLAVE_ADDR  0x00
#define USART_COM_BROADCAST_ADDR 0xFF
#define USART_COM_MASTER_ADDR    0xFE

#define FUNC_READ_ALL_MIN_LENGTH 8
#define FUNC_WRITE_ALL_LENGTH 12
#define FUNC_WRITE_ALL_DATA_LENGTH 7
#define FUNC_WRITE_SSP_DATA_LEN 66

#define PROT_RX_TIMEOUT_US 500000

int protoGetGlobalStats(char* str);
int protoResetGlobalStats(char* str);
void protoInit (void);

typedef union
{
	struct _usartComWriteAllStr
	{
		//START(1)|ADDR(1)|FUNC(1)|WY(2)|DISP(4)|DISP_OPTIONS(1)|CRC(1)|STOP(1)
		volatile uint8_t start;
		volatile uint8_t addr;
		volatile uint8_t func;
		volatile uint8_t wy[2];
		volatile uint8_t disp[4];
		volatile uint8_t disp_options;
		volatile uint8_t crc;
		volatile uint8_t stop;
	} usartComWriteAllStr;

	volatile uint8_t usartComRxFrame[USART_COM_MAXFRAME];
} usartComRxUnionFrame_t;

typedef union
{
	struct _usartComReadBzStr
	{
		//START(1)|ADDR(1)|FUNC(1)|CRC(1)|STOP(1)
		uint8_t start;
		uint8_t addr;
		uint8_t func;
		uint8_t crc;
		uint8_t stop;
	} usartComReadBzStr;

	uint8_t usartComTxFrame[USART_COM_MAXFRAME];
} usartComTxUnionFrame_t;

typedef enum
{
	FUNC_WRITE_ALL=0,
	FUNC_WRITE_BZ,
	FUNC_READ_ALL,
	FUNC_READ_BZ,
	FUNC_WRITE_SSP,
	FUNC_READ_SSP,
	FUNC_WRITE_PRT,
	FUNC_READ_PRT,
	FUNC_READ_ERROR1,
	FUNC_INVALID
} USART_COM_RX_FUNC_t;

typedef enum
{
	WAITING_FOR_START_BYTE = 0,
	WAITING_FOR_ADDR,
	WAITING_FOR_FUNC_CODE,
	WAITING_FOR_WRITE_DATA,
	WAITING_FOR_CRC,
	WAITING_FOR_STOP_BYTE,
	FINISHED,
} USART_COM_RX_STATE_t;

typedef struct
{
	uint32_t usartComPartialFrames;
	uint32_t usartComParssedRxOk;
	uint32_t usartComTxFrames;
	uint32_t usartComFramesToMe;
	uint32_t usartComGoodFramesToMe;
	uint32_t usartComFlushesNr;
	uint32_t usartComFramesBroadcast;
	uint32_t usartComFramesToOthers;
	uint32_t usartComFramesFuncNotSupp;
	uint32_t usartComFramesWaitingForStart;
	uint32_t usartComFramesWaitingForStop;
	uint32_t usartComFramesBedCrc;
	uint32_t usartComBufsWithoutContext;
	uint32_t usartComBytesAfterFinish;
	uint32_t usartComShouldNotHappen;
	uint32_t usartComParssedRxFailed;
	uint32_t usartComTimeout;


	uint32_t readLinuxErr;
	uint32_t selectLinuxErr;
	uint32_t writeLinuxErr;
} usartComCritStats_t;


typedef struct _usartCom_rx
{
	usartComRxUnionFrame_t usartComRxUnionFrame;
	USART_COM_RX_STATE_t usartCom_rx_state;
	USART_COM_RX_FUNC_t usartComFunction;
	uint16_t usartComDataLength;
	bool usartComToMe;
} usartCom_rx_t;

typedef struct _usartCom_tx
{
	usartComTxUnionFrame_t usartComTxUnionFrame;
	uint16_t usartComTxDataLength;
} usartCom_tx_t;
#endif /* DRV_PROTOCOLR_HPP_ */
