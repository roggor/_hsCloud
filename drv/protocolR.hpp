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
//	WAITING_FOR_READ_DATA, //data comming from other slaves, state until data length field
	WAITING_FOR_CRC,
	WAITING_FOR_STOP_BYTE,
	WAITING_FOR_STOP_BYTE_ERROR_CRC,
	FINISHED,
} USART_COM_RX_STATE_t;


typedef struct _usartCom_rx
{
	usartComRxUnionFrame_t usartComRxUnionFrame;
	USART_COM_RX_STATE_t usartCom_rx_state;
	USART_COM_RX_FUNC_t usartComFunction;

	bool usartComToMe;
	uint16_t usartComDataLength;

} usartCom_rx_t;

#endif /* DRV_PROTOCOLR_HPP_ */
