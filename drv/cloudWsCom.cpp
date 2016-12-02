/*
 * cloudWsCom.cpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */

#include "libwsApi.hpp"
#include "messagesJson.hpp"
#include "cloudWsCom.hpp"

/*************************************************************************/
#define DBG_INFO
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

void cloudWsComClass::init(void)
{
	rxThread=std::thread(&cloudWsComClass::rxThreadFunc, this);
	pollThread=std::thread(&cloudWsComClass::pollThreadFunc, this);

	//start lws thread function
	libwsApi_init();
}

void cloudWsComClass::shutdown(void)
{
	libwsApi_shutdown();
	rxThread.join();
	pollThread.join();
}

void cloudWsComClass::rxThreadFunc(void)
{
	char buf[MAX_RX_PAYLOAD];
	unsigned int len;

	while(1)
	{
		len=libwsApi_recv(buf);
		dbg_info ("Client RX:\n%s\n", buf);

		/*
		 * Json::Value root;
		 * Json::Reader reader;
		 *
		 * reader.parse( buf, root );
		 * std::string msgType = root.get("mMessageType", "mMessageTypeErr" ).asString();
		 *
			if (msgType=="regPing")
			{
				PingMsgClass *pingRx = new PingMsgClass("", 0, 0, 0);

				pingRx->DeserStr(msgPayload);
				std::cout << "BB Rx PING seq=" << pingRx->mSeqNr << std::endl;

				delete pingRx;
			}
			else
			{
				std::cout << "BB ERROR Detected PING" << std::endl;
			}
		 *
		 */
	}
}

void cloudWsComClass::pollThreadFunc(void)
{
	PingMsgClass *pingTx;
	int retSend;

	while(1)
	{
		waitGridDelay(2000);

		pingTx=new PingMsgClass(0,0,0);

		std::string pingTxDeserStr;
		pingTx->SerStr(pingTxDeserStr);

		retSend=libwsApi_send(pingTxDeserStr.c_str(), pingTxDeserStr.length());

		if(!retSend)
		{
			dbg_info("Client TX:\n%s\n", pingTxDeserStr.c_str());
		}
		else
		{
			dbg_err("Client TX failed\n");
		}
	}
}

#define MAX_TIME_SYNC_DEVIATION  50
/** Wait the time in ms to next time aligned time grid. */
void cloudWsComClass::waitGridDelay(unsigned long ulInterval)
{
	struct timeval tv;
	unsigned long  ulDelay;
	unsigned long  ulMilliSecOfDay;

	gettimeofday(&tv,(struct timezone*)0);
	ulMilliSecOfDay = (tv.tv_sec%86400)*1000 + (tv.tv_usec/1000);
	ulDelay         = ulInterval - (ulMilliSecOfDay % ulInterval);

	if (ulDelay <= MAX_TIME_SYNC_DEVIATION)
		ulDelay += ulInterval;

	tv.tv_sec  =  ulDelay / 1000;                   /*seconds*/
	tv.tv_usec = (ulDelay  - tv.tv_sec*1000)*1000;  /*u seconds*/
	select(0,NULL,NULL,NULL,&tv);
}
