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
#include <string.h>

#include <queue>

#include "globConfs.hpp"
#include "protocolR.hpp"

/*************************************************************************/
//#define DBG_INFO
#define DBG_ERR

#undef dbg_info
#ifdef DBG_INFO
#define dbg_info(fmt, args...) printf("INFO %s(): " fmt, __func__, ##args)
#else
#define dbg_info(fmt, args ...)
#endif

#undef dbg_err
#ifdef DBG_ERR
#define dbg_err(fmt, args...) printf("ERR %s(): " fmt, __func__, ##args)
#else
#define dbg_err(...)
#endif
/*************************************************************************/

static int fd;
static usartCom_rx_t usartCom_rx={0, };
std::queue<usartCom_tx_t> usartCom_tx;
static usartComCritStats_t usartComCritStats = {0, };

static int canTx;
static pthread_mutex_t mutexCnt;
static pthread_cond_t  condUsartCanTx;

static void *rxRs485Thread(void *arg);
static void *txRs485Thread(void *arg);
static int parseRxProtocol(unsigned char *buff, unsigned int length, bool flush);


int protoGetGlobalStats(char* str)
{
	/* TODO: any locking needed? */

#define STATS(__stat__,__str__) ret=sprintf(p, "%s %u\n",__str__,__stat__); p=p+ret;

	char *p=str;
	unsigned int ret;

	ret=sprintf(p, "Protocol get stats [dev=%s]:\n", PROTO_R_DEV_NAME); p=p+ret;
	STATS(usartComCritStats.usartComTxFrames,              "TxFrames                 :")

	STATS(usartComCritStats.usartComFramesToMe,            "FramesToMe               :")
	STATS(usartComCritStats.usartComGoodFramesToMe,        "GoodFramesToMe           :")
	STATS(usartComCritStats.usartComFramesToOthers,        "FramesToOthers(0)        :")
	STATS(usartComCritStats.usartComFramesBroadcast,       "FramesBroadcast(0)       :")
	STATS(usartComCritStats.usartComGoodFramesToOthers,    "GoodFramesToOthers(0)    :")
	STATS(usartComCritStats.usartComTimeout,               "Timeout(0)               :")

	STATS(usartComCritStats.usartComFramesFuncNotSupp,     "FramesFuncNotSupp(0)     :")
	STATS(usartComCritStats.usartComFramesWaitingForStart, "FramesWaitingForStart(0) :")
	STATS(usartComCritStats.usartComFramesWaitingForStop,  "FramesWaitingForStop(0)  :")
	STATS(usartComCritStats.usartComFramesBedCrc,          "FramesBedCrc(0)          :")
	STATS(usartComCritStats.usartComShouldNotHappen,       "ShouldNotHappen(0)       :")

	ret=sprintf(p, "\n"); p=p+ret;
	ret=sprintf(p, "Linux errors:\n"); p=p+ret;
	STATS(usartComCritStats.readLinuxErr,                  "readLinuxErr(default=0)          :")
	STATS(usartComCritStats.selectLinuxErr,                "selectLinuxErr(default=0)        :")
	STATS(usartComCritStats.writeLinuxErr,                 "writeLinuxErr(default=0)         :")
	return (p - str);
#undef STATS
}

int protoResetGlobalStats(char* str)
{
	/* TODO: any locking needed? */
	char *p=str;
	unsigned int ret;

	ret=sprintf(p, "Protocol reset stats [dev=%s]:\n", PROTO_R_DEV_NAME); p=p+ret;
	memset(&usartComCritStats, 0, sizeof(usartComCritStats));

	return (p - str);
}

static uint8_t inline usartComCRC (volatile uint8_t* frame, uint16_t len)
{
	uint8_t ret = 0xff;
	uint16_t k=0;

	for(k=0; k<len; k++)
		ret ^= frame[k];

	return ret;
}

void protoInit (void)
{
	int res;
	pthread_t rx_thread;
	pthread_t tx_thread;
	struct termios newtio;

	canTx=0;
	res = pthread_mutex_init(&mutexCnt, NULL);
	pthread_cond_init(&condUsartCanTx, NULL);
	if (res != 0) {
		dbg_err("mutex initialization failed");
		exit(EXIT_FAILURE);
	}

	fd = open(PROTO_R_DEV_NAME, O_RDWR | O_NOCTTY );
	if (fd < 0)
	{
		dbg_err("open %s failed\n", PROTO_R_DEV_NAME);
		exit(EXIT_FAILURE);
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

	res = pthread_create(&rx_thread, NULL, rxRs485Thread, NULL);
	if (res != 0) {
		dbg_err("rx pthread_create failed\n");
		exit(EXIT_FAILURE);
	}

	res = pthread_create(&tx_thread, NULL, txRs485Thread, NULL);
	if (res != 0) {
		dbg_err("tx pthread_create failed\n");
		exit(EXIT_FAILURE);
	}
}

static void *rxRs485Thread(void *arg)
{
	int retSelect, retRead, parseRet;
	unsigned char buff[1];
	fd_set set;
	long timeoutUs = PROT_RX_TIMEOUT_US;
	struct timeval timeout;
	int retFlush;

	//read
	while (1)
	{
		FD_ZERO(&set);
		FD_SET(fd, &set);
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutUs;

		retSelect=select(fd + 1, &set, NULL, NULL, &timeout);
		if(retSelect < 0)
		{
			usartComCritStats.selectLinuxErr++;
			dbg_err("selectLinuxErr\n");

			timeoutUs = PROT_RX_TIMEOUT_US;
		}
		else if(retSelect == 0)
		{
			//timeout, only clean global variables in parseRxProtocol, and return rx_state
			retFlush=parseRxProtocol(NULL, 0, true);
			usartComCritStats.usartComTimeout++;
			dbg_info("timeout: rx_state=%u\n", retFlush);

			timeoutUs = PROT_RX_TIMEOUT_US;
		}
		else
		{
			//sprawdzic czy read nie zwroci bledu
			retRead=read(fd, buff, sizeof(buff));
			if(retRead <= 0)
			{
				dbg_err ("readLinuxErr\n");
				usartComCritStats.readLinuxErr++;

				timeoutUs = PROT_RX_TIMEOUT_US;
				continue;
			}

			parseRet=parseRxProtocol(buff, retRead, false);
			if (parseRet<0)
			{
				retFlush=parseRxProtocol(NULL, 0, true);
				dbg_err("parse RX critical, rx_state=%u\n", retFlush);

				timeoutUs = PROT_RX_TIMEOUT_US;
			}
			else if (!parseRet)
			{
				//nie odebrano żadnej całej ramki, zmienić timeout selecta
				timeoutUs = PROT_RX_TIMEOUT_US - timeout.tv_usec;
				if (timeoutUs<0)
					timeoutUs=0;
			}
			else
			{
				//odebrano przy najmniej jedną całą ramkę, nie updatuje timeoutu
				timeoutUs = PROT_RX_TIMEOUT_US;
			}
		}
	}
}

static void *txRs485Thread(void *arg)
{
	int ret;
	unsigned int written;
	usartCom_tx_t txFrame;
	//write
	while (1)
	{
		written=0;

		pthread_mutex_lock(&mutexCnt);
		while (canTx<=0)
			pthread_cond_wait(&condUsartCanTx, &mutexCnt);

		if (usartCom_tx.empty())
		{
			txFrame.usartComTxUnionFrame.usartComTxFrame[0]=USART_COM_STARTBYTE;
			txFrame.usartComTxUnionFrame.usartComTxFrame[1]=USART_COM_MASTER_ADDR;
			txFrame.usartComTxUnionFrame.usartComTxFrame[2]=FUNC_READ_BZ;
			txFrame.usartComTxUnionFrame.usartComTxFrame[3]=usartComCRC(txFrame.usartComTxUnionFrame.usartComTxFrame, 3);
			txFrame.usartComTxUnionFrame.usartComTxFrame[4]=USART_COM_STOPTBYTE;

			txFrame.usartComTxDataLength=5;
		}
		else
		{
			//tu pewnie nie zadziala, moze jakies memcpy
			txFrame=usartCom_tx.front();
			usartCom_tx.pop();
		}

		do
		{
			ret=write(fd, (void*)(((unsigned char*)(txFrame.usartComTxUnionFrame.usartComTxFrame)) + written), (txFrame.usartComTxDataLength-written));
			if (ret <= 0)
			{
				dbg_err("writeLinuxErr");
				usartComCritStats.writeLinuxErr++;
				break;
			}
			else
				written=written+ret;
		}
		while (written!=txFrame.usartComTxDataLength);

		if (ret > 0)
			usartComCritStats.usartComTxFrames++;

		tcdrain(fd);

		canTx--;
		pthread_mutex_unlock(&mutexCnt);
	}
}

/*
 * return:
 * 0  - at least one complete frame received/flush
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
	int ret = 0;
	uint8_t usartComRxByte;
	USART_COM_RX_STATE_t flushRxState;

	if (flush)
	{
		flushRxState=usartCom_rx.usartCom_rx_state;
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
				usartComCritStats.usartComFramesWaitingForStart++;
			break;

		case WAITING_FOR_ADDR:
			if (usartComRxByte == USART_COM_SLAVE_ADDR)
			{
				usartComCritStats.usartComFramesToMe++;
				usartCom_rx.usartComToMe=true;
			}
			else
			{
				if (usartComRxByte == USART_COM_BROADCAST_ADDR)
					usartComCritStats.usartComFramesBroadcast++;
				else
					usartComCritStats.usartComFramesToOthers++;

				usartCom_rx.usartComToMe=false;
			}
			usartCom_rx.usartCom_rx_state = WAITING_FOR_FUNC_CODE;
			break;

		case WAITING_FOR_FUNC_CODE:
			switch (usartComRxByte)
			{
			case FUNC_WRITE_ALL:
				usartCom_rx.usartCom_rx_state = WAITING_FOR_DATA;
				usartCom_rx.usartComFunction = FUNC_WRITE_ALL;
				break;
				/*
				 * add new functions
				 */

			default:
				usartComCritStats.usartComFramesFuncNotSupp++;
				usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
				usartCom_rx.usartComDataLength=0;
				usartCom_rx.usartComFunction = FUNC_INVALID;
				break;
			}
			break;

		case WAITING_FOR_DATA: // START(1)|ADD(1)|FUNC(1)|WY(2)|DISP(4)|DISP_OPTIONS(1)|CRC(1)|STOP(1)
			switch (usartCom_rx.usartComFunction)
			{
			case FUNC_WRITE_ALL:
				if (usartCom_rx.usartComDataLength >= (FUNC_WRITE_ALL_LENGTH-3))
					usartCom_rx.usartCom_rx_state=WAITING_FOR_CRC;
				break;
				/*
				 * add new functions
				 */

			default:
				usartComCritStats.usartComShouldNotHappen++;
				return (-1);
			}
			break;

		case WAITING_FOR_CRC:
			if (usartComRxByte==usartComCRC(usartCom_rx.usartComRxUnionFrame.usartComRxFrame, usartCom_rx.usartComDataLength))
				usartCom_rx.usartCom_rx_state=WAITING_FOR_STOP_BYTE;
			else
			{
				usartComCritStats.usartComFramesBedCrc++;
				usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
				usartCom_rx.usartComDataLength=0;
				usartCom_rx.usartComFunction = FUNC_INVALID;
			}
			break;

		case WAITING_FOR_STOP_BYTE:
			if (usartComRxByte == USART_COM_STOPTBYTE)
			{
				usartCom_rx.usartComRxUnionFrame.usartComRxFrame[usartCom_rx.usartComDataLength]=usartComRxByte;
				usartCom_rx.usartComDataLength++;

#if 0
				printf ("RCVED: ");
				int k;
				for (k=0; k< usartCom_rx.usartComDataLength; k++)
					printf("%2x ", usartCom_rx.usartComRxUnionFrame.usartComRxFrame[k]);
				printf ("\n");
#endif

				if(usartCom_rx.usartComToMe)
				{
					//resetting timeout only if good frames to me
					ret=1;

					pthread_mutex_lock(&mutexCnt);
					canTx++;
					pthread_cond_signal(&condUsartCanTx);
					pthread_mutex_unlock(&mutexCnt);

					usartComCritStats.usartComGoodFramesToMe++;
					switch (usartCom_rx.usartComFunction)
					{
					case FUNC_WRITE_ALL:
						dbg_info("service FUNC_WRITE_ALL\n");
						break;
						/*
						 * add new functions
						 */

					default:
						usartComCritStats.usartComShouldNotHappen++;
						return (-1);
					}
				}
				else
				{
					//don't reset timeout (ret=0)
					usartComCritStats.usartComGoodFramesToOthers++;
				}
			}
			else
				usartComCritStats.usartComFramesWaitingForStop++;

			usartCom_rx.usartCom_rx_state = WAITING_FOR_START_BYTE;
			usartCom_rx.usartComDataLength = 0;
			usartCom_rx.usartComFunction = FUNC_INVALID;
			break;

		default:
			usartComCritStats.usartComShouldNotHappen++;
			return (-1);
		} //END of switch (usartCom_rx.usartCom_rx_state)

		if ((usartCom_rx.usartCom_rx_state != WAITING_FOR_START_BYTE))
		{
			usartCom_rx.usartComRxUnionFrame.usartComRxFrame[usartCom_rx.usartComDataLength]=usartComRxByte;
			usartCom_rx.usartComDataLength++;
		}
	} //END of for (l=0; l<length; l++)
	return ret;
}
