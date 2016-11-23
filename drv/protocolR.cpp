/*
 * protocolR.cpp
 *
 *  Created on: Nov 8, 2016
 *      Author: rogal
 */
#include <stdio.h>
#include <pthread.h>
#include <termios.h> //for serial configuration
#include <linux/serial.h> //for rs485 configuration
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "globConfs.hpp"
#include "protocolR.hpp"

static int fd;
static usartCom_rx_t usartCom_rx={0, };

static void *rxRs485Thread(void *arg);
static int parseRxProtocol(unsigned char *buff, unsigned int length, bool flush);


void protoInit (void)
{
	int res;
	pthread_t rx_thread;
	struct termios newtio;

	fd = open(PROTO_R_DEV_NAME, O_RDWR | O_NOCTTY );
	if (fd < 0)
	{
		perror(PROTO_R_DEV_NAME);
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 1;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0; //255;   /* blocking read until 255 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

#if 0
	/* RS485 ioctls: */
#define TIOCGRS485      0x542E
#define TIOCSRS485      0x542F

	struct serial_rs485 rs485conf;

	/* Enable RS485 mode: */
	rs485conf.flags |= SER_RS485_ENABLED;

	/* Set logical level for RTS pin equal to 1 when sending: */
	rs485conf.flags |= SER_RS485_RTS_ON_SEND;

	/* Set logical level for RTS pin equal to 0 after sending: */
	rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

	/* Set rts delay before send, if needed: */
	rs485conf.delay_rts_before_send = 10;

	/* Set rts delay after send, if needed: */
	rs485conf.delay_rts_after_send = 10;

	/* Set this flag if you want to receive data even whilst sending data */
	//rs485conf.flags |= SER_RS485_RX_DURING_TX;

	if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
		/* Error handling. See errno. */
	}
#endif

	res = pthread_create(&rx_thread, NULL, rxRs485Thread, NULL);
	if (res != 0) {
		printf("Error pthread_create\n");
		exit(EXIT_FAILURE);
	}
}

static void *rxRs485Thread(void *arg)
{
	int retSelect, retRead, parseRet;
	unsigned char buff[255];
	fd_set set;
	struct timeval timeout;
	int retFlush;

	//read
	while (1)
	{
		//sprawdzic czy read nie zwroci bledu
		retRead=read(fd, buff, sizeof(buff));
		if(retRead <= 0)
		{
			printf ("readLinuxErr++\n");
			//usartComCritStats.readLinuxErr++;
			continue;
		}

		parseRet=parseRxProtocol(buff, retRead, false);
		if (!parseRet)
		{
			printf("parseRxProtocol==0 OK\n");
			//protoStatsIncr(pArgs->ttyNr, RECV_OK);
			break;
		}
		else if (parseRet<0)
		{
			printf("parseRxProtocol<0 error\n");
			//protoStatsIncr(pArgs->ttyNr, RECV_ERR);

			usleep(20000); //17ms to jest 255 bajtow@115200
			tcflush(fd, TCIFLUSH);

			//only clean global variables in parseRxProtocol, and return rx_state
			retFlush=parseRxProtocol(NULL, 0, true);
			//printf("parseRxError: addr=0x%x,rx_state=%u\n", usartCom_tx.addrReq, retFlush);
			break;
		}
		else
		{
			printf("parseRxProtocol > 0; partial OK\n");
			//protoStatsIncr(pArgs->ttyNr, RECV_PARTIAL);
		}
	}
}

static uint8_t inline usartComCRC (volatile uint8_t* frame, uint16_t len)
{
	uint8_t ret = 0xff;
	uint16_t k=0;

	for(k=0; k<len; k++)
		ret ^= frame[k];

	return ret;
}

/*
 * return:
 * 0  - completed frame received/flush
 * 1  - partial frame received
 * -1 - error during receiving
 * -2 - frames without context
 *
 * if(flush==true) to zwraca rxState
 *
 */
static int parseRxProtocol(unsigned char *buff, unsigned int length, bool flush)
{
	unsigned int l;
	uint8_t usartComRxByte;
	USART_COM_RX_STATE_t flushRxState;

	if (flush)
	{
		flushRxState=usartCom_rx.usartCom_rx_state;
		//usartComCritStats.usartComFlushesNr++;
		printf("usartComCritStats.usartComFlushesNr++\n");
		usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
		usartCom_rx.usartComDataLength=0;
		usartCom_rx.usartComFunction = FUNC_INVALID;
		return ((int) flushRxState);
	}

	for (l=0; l<length; l++)
	{
		usartComRxByte = (unsigned char) buff[l];

		switch (usartCom_rx.usartCom_rx_state)
		{
		case WAITING_FOR_START_BYTE:
			if (usartComRxByte == USART_COM_STARTBYTE)
				usartCom_rx.usartCom_rx_state = WAITING_FOR_ADDR;
			else
			{
				printf("usartComCritStats.usartComFramesWaitingForStart++\n");
				//usartComCritStats.usartComFramesWaitingForStart++;
			}
			break;

		case WAITING_FOR_ADDR:
			if (usartComRxByte == USART_COM_SLAVE_ADDR)
			{
				printf("usartComCritStats.usartComFramesToMe++\n");
				//usartComCritStats.usartComFramesToMe++;
				usartCom_rx.usartCom_rx_state = WAITING_FOR_FUNC_CODE;
			}
			else if (usartComRxByte == USART_COM_BROADCAST_ADDR)
			{
				printf("usartComCritStats.usartComFramesBroadcast++\n");
				//usartComCritStats.usartComFramesBroadcast++;
				goto ERROR;
			}
			else
			{
				printf("usartComCritStats.usartComFramesToOthers++\n");
				//usartComCritStats.usartComFramesToOthers++;
				goto ERROR;
			}
			break;

		case WAITING_FOR_FUNC_CODE:
			switch (usartComRxByte)
			{
			case FUNC_WRITE_ALL:
				usartCom_rx.usartCom_rx_state = WAITING_FOR_WRITE_DATA;
				usartCom_rx.usartComFunction = FUNC_WRITE_ALL;
				break;

			default:
				printf("usartComCritStats.usartComFramesFuncNotSupp++\n");
				//usartComCritStats.usartComFramesFuncNotSupp++;
				goto ERROR;
			}
			break;

		case WAITING_FOR_WRITE_DATA: // START(1)|ADD(1)|FUNC(1)|WY(2)|DISP(4)|DISP_OPTIONS(1)|CRC(1)|STOP(1)
			switch (usartCom_rx.usartComFunction)
			{
			case FUNC_WRITE_ALL:
				if (usartCom_rx.usartComDataLength >= (FUNC_WRITE_ALL_LENGTH-3))
					usartCom_rx.usartCom_rx_state=WAITING_FOR_CRC;
				break;
			default:
				printf("usartCom_rx.usartComCritStats.usartComShouldNotHappen++\n");
				//usartComCritStats.usartComShouldNotHappen++;
				goto ERROR;
			}
			break;

		case WAITING_FOR_CRC:
			if (usartComRxByte==usartComCRC(usartCom_rx.usartComRxUnionFrame.usartComRxFrame, usartCom_rx.usartComDataLength))
				usartCom_rx.usartCom_rx_state=WAITING_FOR_STOP_BYTE;
			else
			{
				printf("usartComCritStats.usartComFramesBedCrc++\n");
				//usartComCritStats.usartComFramesBedCrc++;
				usartCom_rx.usartCom_rx_state=WAITING_FOR_STOP_BYTE_ERROR_CRC;
			}
			break;

		case WAITING_FOR_STOP_BYTE_ERROR_CRC:
			if (usartComRxByte == USART_COM_STOPTBYTE)
			{
				printf("only bed CRC, stop OK\n");
				usartCom_rx.usartCom_rx_state=WAITING_FOR_STOP_BYTE;
			}
			else
			{
				printf("usartComCritStats.usartComFramesWaitingForStop++\n");
				//usartComCritStats.usartComFramesWaitingForStop++;
				goto ERROR;
			}
			break;

		case WAITING_FOR_STOP_BYTE:
			if (usartComRxByte == USART_COM_STOPTBYTE)
			{
				usartCom_rx.usartComRxUnionFrame.usartComRxFrame[usartCom_rx.usartComDataLength]=usartComRxByte;
				usartCom_rx.usartComDataLength++;

				printf("usartComCritStats.usartComGoodFramesToMe++\n");
				//usartComCritStats.usartComGoodFramesToMe++;

#if 0
				printf ("RCVED: ");
				int k;
				for (k=0; k< usartCom_rx.usartComDataLength; k++)
					printf("%2x ", usartCom_rx.usartComRxUnionFrame.usartComRxFrame[k]);
				printf ("\n");
#endif

				usartCom_rx.usartCom_rx_state = FINISHED;
			}
			else
			{
				printf("usartComCritStats.usartComFramesWaitingForStop++\n");
				//usartComCritStats.usartComFramesWaitingForStop++;
				goto ERROR;
			}
			break;

		case FINISHED:
			//bajty za bajtem STOPu sa ignorowane, konczymy return 0
			printf("usartComCritStats.usartComBytesAfterFinish++\n");
			//usartComCritStats.usartComBytesAfterFinish++;
			break;

		default:
			printf("usartComCritStats.usartComShouldNotHappen++\n");
			//usartComCritStats.usartComShouldNotHappen++;
			goto ERROR;

		} //END of switch (usartCom_rx.usartCom_rx_state)

		if ((usartCom_rx.usartCom_rx_state != WAITING_FOR_START_BYTE) &&
				(usartCom_rx.usartCom_rx_state != FINISHED))
		{
			usartCom_rx.usartComRxUnionFrame.usartComRxFrame[usartCom_rx.usartComDataLength]=usartComRxByte;
			usartCom_rx.usartComDataLength++;
		}

	} //END of for (l=0; l<length; l++)

	//przychodza bajty, a my czekamy na bajt startu, bez tego po odebraniu 1 jakiegos bajtu zwracalibysmy "partial frame"
	if (usartCom_rx.usartCom_rx_state == WAITING_FOR_START_BYTE)
	{
		printf("usartComCritStats.usartComBufsWithoutContext++\n");
		//usartComCritStats.usartComBufsWithoutContext++;
		return -2;
	}

	//niedokonczone ramki
	if (usartCom_rx.usartCom_rx_state != FINISHED)
	{
		printf("usartComCritStats.usartComPartialFrames++\n");
		//usartComCritStats.usartComPartialFrames++;
		return 1;
	}
	else
	{
		printf("usartComCritStats.usartComFramesCompleted++\n");
		//usartComCritStats.usartComFramesCompleted++;
		usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
		usartCom_rx.usartComDataLength=0;
		usartCom_rx.usartComFunction = FUNC_INVALID;
		return 0;
	}

	ERROR:
	usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
	usartCom_rx.usartComDataLength=0;
	usartCom_rx.usartComFunction = FUNC_INVALID;
	return -1;
}


