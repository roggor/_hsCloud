/*
 * bbRegistration.cpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"

#include <iostream>
#include <unistd.h>

#include "messagesJson.hpp"
#include "bbRegistration.hpp"

/* RPC style rabbit
 *
 * INIT:
 * 1. Lunch the thread to rx responses from Cloud
 * 2. Lunch the thread to tx requests to Cloud
 *
 *
 * Tx thread
 * -opens it's own tcp, socket, connection, queue
 * -initially it should run, every 1s it sends PING request
 * -then signals Rx thread that it should go now
 * -and falling a sleep. It should wait for 1s to past AND signal from Rx task
 * -LOGIC during send:
 * 	+inserts seq number, return queue name, registration state
 * -API needed
 * 	+create logic channel (tcp, socket, connection, queue)
 * 	+send JSON string to logic channel
 *
 * Rx thread
 * -opens it's own tcp, socket, connection, queue
 * -initially it will not run, must wait for tx thread to wake it
 * -when woken then it consumes (with timeout) from rabbit
 * -after succesfull consuming(or timeout) it signals Tx thread
 * -LOGIC during comsuming:
 * 	+checks seq number, detects gaps, duplications
 * 	+gets timsync
 * 	+gets reg state
 *
 * -API needed
 *  +logic channel
 * 	+block with timeout on consuming
 *
 * COMMON variables:
 * 	-regState, seqNumber, queueName,
 * 	-conditional variables, mutexes
 *
 */

void bbRegistrationClass::regResRxThreadFunc(void)
{
	while(1)
	{
		{
			amqp_envelope_t envelope;

			std::unique_lock<std::mutex> lock(txRxMutex);
			while(!canRx)
				rxCondVar.wait(lock);
			canRx=false;

			rbMqReceive(connRx, queuenameRx, &envelope);

			std::string msgPayload(static_cast<char*>(envelope.message.body.bytes));
			Json::Value root;
			Json::Reader reader;

			reader.parse( msgPayload.c_str(), root );
			std::string msgType = root.get("mMessageType", "mMessageTypeErr" ).asString();

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

			amqp_destroy_envelope(&envelope);

			canTx=true;
			txCondVar.notify_one();
		}
	}
}

void bbRegistrationClass::regReqTxThreadFunc(void)
{
	PingMsgClass *pingTx;

	while(1)
	{
		{
			std::unique_lock<std::mutex> lock(txRxMutex);
			while(!canTx)
				txCondVar.wait(lock);
			canTx=false;

			pingTx=new PingMsgClass(RB_MQ_BBREG_RES_QUEUE_UNIQUE_ID, txSeqNumber, txRegState, 0);

			std::string pingTxDeserStr;
			pingTx->SerStr(pingTxDeserStr);
			std::cout << "BBreg Tx PING seq=" << pingTx->mSeqNr << std::endl;
			rbMqSend(connTx, RB_MQ_DIRECT_EXCHANGE, RB_MQ_BBREG_REQ_KEY, pingTxDeserStr.c_str());

			delete pingTx;

			canRx=true;
			rxCondVar.notify_one();
		}
		txSeqNumber++;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void bbRegistrationClass::init(void)
{
	canTx=true;
	canRx=false;

	//Lunch the thread to rx responses from Cloud
	connRx=rbMQConnect(RB_MQ_PORT, RB_MQ_SERVER_NAME);
	queuenameRx=rbMqDeclareQueue(connRx, RB_MQ_BBREG_RES_QUEUE_UNIQUE_ID, RB_MQ_DIRECT_EXCHANGE, RB_MQ_BBREG_RES_KEY_UNIQUE_ID);
	rbMqBasicConsume(connRx, queuenameRx);
	regResRxThread=std::thread(&bbRegistrationClass::regResRxThreadFunc, this);

	//Lunch the thread to tx requests to Cloud
	connTx=rbMQConnect(RB_MQ_PORT, RB_MQ_SERVER_NAME);
    regReqTxThread=std::thread(&bbRegistrationClass::regReqTxThreadFunc, this);
}

void bbRegistrationClass::shutdown(void)
{
	regResRxThread.join();
	rbMqDisconnect(connTx);
	regReqTxThread.join();
	rbMqDisconnect(connRx);
}




