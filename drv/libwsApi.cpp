/*
 * cWsApi.cpp
 *
 *  Created on: Nov 8, 2016
 *      Author: rogal
 */

#include "globConfs.hpp"

#include <signal.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "libwebsockets.h"
#include "libwsApi.hpp"

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

static void lwsThreadFunc(void);
static int callbackEcho(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

struct txDataStruct
{
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + MAX_TX_PAYLOAD + LWS_SEND_BUFFER_POST_PADDING];
	unsigned int len;
	bool txBusy;

	std::mutex txMutex;
};

struct rxDataStruct
{
	unsigned char buf[MAX_RX_PAYLOAD];
	unsigned int len;
	unsigned int rxCount;

	std::mutex rxMutex;
	std::condition_variable rxCondVar;
};

enum connectionStateT
{
	NOT_CONNECTED,
	CONNECTING,
	ESTABLISHED
};

static std::thread lwsThread;
static int force_exit;
static connectionStateT connState=NOT_CONNECTED;
static struct txDataStruct txData={0, };
static struct rxDataStruct rxData={0, };

struct lws *gWsi=NULL;
struct lws_context *context;
static struct lws_protocols protocols[2]=
{
		{"callbackEcho" , callbackEcho , sizeof(struct txDataStruct) },
		{ NULL     , NULL          , 0 }
};

void libwsApi_init(void)
{
	connState = NOT_CONNECTED;
	lwsThread=std::thread(&lwsThreadFunc);
}

int libwsApi_send(char *pData, int dataLen)
{
	if (connState != ESTABLISHED)
	{
		dbg_info("waiting for connection to establish, not sending\n");
		return -1;
	}

	if (dataLen<=MAX_TX_PAYLOAD)
	{
		std::unique_lock<std::mutex> lock(txData.txMutex);
		if (!txData.txBusy)
		{
			memcpy(&txData.buf[LWS_SEND_BUFFER_PRE_PADDING], pData, dataLen);
			txData.len=dataLen;
			txData.txBusy=true;

			lock.unlock();

			return 0;
		}
		else
		{
			lock.unlock();
			dbg_err("txBusy == true\n");
		}
		//lepiej nie bo wtedy kilka watków może sie do biblioteki dobierac
		//		lws_callback_on_writable(gWsi);
	}
	else
		dbg_err("dataLen > MAX_TX_PAYLOAD\n");


	return -1;
}

int libwsApi_recv(char *pData)
{
	{
		std::unique_lock<std::mutex> lock(rxData.rxMutex);
		while(rxData.rxCount <= 0)
			rxData.rxCondVar.wait(lock);

		rxData.rxCount--;
		memcpy(pData, rxData.buf, rxData.len);
	}

	return rxData.len;
}

void libwsApi_shutdown(void)
{
	lwsThread.join();
}

void sighandler(int sig)
{
	force_exit = 1;
}

void lwsThreadFunc(void)
{
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);
	info.port = -1; //listen port for client
	info.iface = WS_ETH_IF;
	info.protocols = protocols;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;
	info.gid = -1;
	info.uid = -1;
	info.options = 0;

	int port=WS_SERVER_PORT;
	char address[256]=WS_SERVER_IP;
	char ads_port[256 + 30];
	char uri[] = WS_SERVER_URI;

	signal(SIGINT, sighandler);

	//Create context via info
	dbg_info("create context(%s) %s %s%s:%d\n", protocols[0].name, info.iface, address, uri, port);
	context = lws_create_context(&info);
	if (context == NULL) {
		dbg_err("libwebsocket init failed\n");
	}

	address[sizeof(address) - 1] = '\0';
	sprintf(ads_port, "%s:%u", address, port & 65535);

	while (!force_exit)
	{
		if (connState==NOT_CONNECTED)
		{
			dbg_info("State==NOT_CONNECTED, now connecting...\n");
			gWsi=lws_client_connect(context, address, port, 2, uri, ads_port, ads_port, NULL, -1);
			if (!gWsi) {
				dbg_err("Client failed to connect to %s:%u\n", address, port);
				sleep(1);
			}
			else
			{
				dbg_info("Client connected to %s%s:%d\n", address, uri, port);
				connState=CONNECTING;
			}
		}

		lws_callback_on_writable(gWsi);
		lws_service(context, 10);
	}

	lws_context_destroy(context);
}

int callbackEcho(struct lws *wsi, enum lws_callback_reasons reason, void *user,
	      void *in, size_t len)
{
	int n = 0;
	struct txDataStruct *pTxData = &txData;

/*
	if ((reason!=LWS_CALLBACK_CLOSED) &&
		(reason!=LWS_CALLBACK_CLIENT_CONNECTION_ERROR) &&
		(reason!=LWS_CALLBACK_CLIENT_ESTABLISHED) &&
		(reason!=LWS_CALLBACK_CLIENT_RECEIVE) &&
		(reason!=LWS_CALLBACK_CLIENT_WRITEABLE) &&
		(reason!=LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS) &&
		(reason!=LWS_CALLBACK_GET_THREAD_ID) &&
		(reason!=LWS_CALLBACK_CHANGE_MODE_POLL_FD) &&
		(reason!=LWS_CALLBACK_LOCK_POLL) &&
		(reason!=LWS_CALLBACK_UNLOCK_POLL))

		printf("C= %d\n", reason);
*/

	switch (reason) {
	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		dbg_err("Connection closed, reason=%d\n", (int)reason);
		connState = NOT_CONNECTED;
		gWsi=NULL;
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		dbg_info("Client connection established\n");
		connState = ESTABLISHED;

		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			dbg_info ("Client RX:\n%s\n", in);

			std::unique_lock<std::mutex> lock(rxData.rxMutex);

			memcpy(rxData.buf, in, len);
			rxData.len=len;
			rxData.rxCount++;

			rxData.rxCondVar.notify_one();
		}

		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
			std::unique_lock<std::mutex> lock(txData.txMutex);
			if (pTxData->txBusy==true)
			{
				dbg_info ("Client TX:\n%s\n", &pTxData->buf[LWS_SEND_BUFFER_PRE_PADDING]);
				n = lws_write(wsi, &pTxData->buf[LWS_SEND_BUFFER_PRE_PADDING], pTxData->len, LWS_WRITE_TEXT);
				if (n < 0)
					dbg_err("%d writing to socket, hanging up\n", n);

				if (n < (int)pTxData->len)
					dbg_err("Partial write\n");
				else
					txData.txBusy=false;
			}
		}

		/* get notified as soon as we can write again */
		//lws_callback_on_writable(wsi);

		break;

	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
		break;

	default:
		break;
	}
	return 0;
}

