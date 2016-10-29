/*
 * mainCloudSim.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"

#include <iostream>

#include "rabbitDrv.hpp"
#include "messagesJson.hpp"
#include "cloudServerSim.hpp"

void cloudServerSimClass::init(void)
{
	canSimTx=false;
	canSimRx=true;

	//Lunch the thread to rx responses from BB
	connSimRx=rbMQConnect(RB_MQ_PORT, RB_MQ_SERVER_NAME);
	queuenameSimRx=rbMqDeclareQueue(connSimRx, RB_MQ_BBREG_REQ_QUEUE, RB_MQ_DIRECT_EXCHANGE, RB_MQ_BBREG_REQ_KEY);
	rbMqBasicConsume(connSimRx, queuenameSimRx);
	simRxThread=std::thread(&cloudServerSimClass::simRxThreadFunc, this);

	//Lunch the thread to tx requests to BB
	connSimTx=rbMQConnect(RB_MQ_PORT, RB_MQ_SERVER_NAME);
	simTxThread=std::thread(&cloudServerSimClass::simTxThreadFunc, this);
}

void cloudServerSimClass::shutdown(void)
{
	simRxThread.join();
	rbMqDisconnect(connSimRx);
	simTxThread.join();
	rbMqDisconnect(connSimTx);
}

void cloudServerSimClass::simRxThreadFunc(void)
{
	while(1)
	{
		{
			amqp_envelope_t envelope;
			std::unique_lock<std::mutex> lock(simTxRxMutex);
			while(!canSimRx)
				simRxCondVar.wait(lock);
			canSimRx=false;

			rbMqReceive(connSimRx, queuenameSimRx, &envelope);

			std::string msgPayload(static_cast<char*>(envelope.message.body.bytes));
			Json::Value root;
			Json::Reader reader;

			reader.parse( msgPayload.c_str(), root );
			std::string msgType = root.get("mMessageType", "mMessageTypeErr" ).asString();

			if (msgType=="regPing")
			{
				PingMsgClass *pingSimRx=new PingMsgClass("", 0, 0, 0);

				pingSimRx->DeserStr(msgPayload);
				std::cout << "CloudReg Rx PING seq=" << pingSimRx->mSeqNr << std::endl;

				delete pingSimRx;
			}
			else
			{
				std::cout << "CloudReg ERROR Detected PING" << std::endl;
			}

			amqp_destroy_envelope(&envelope);

			canSimTx=true;
			simTxCondVar.notify_one();
		}
	}
}

void cloudServerSimClass::simTxThreadFunc(void)
{
	PingMsgClass *pingSimTx;

	while(1)
	{
		{
			std::unique_lock<std::mutex> lock(simTxRxMutex);
			while(!canSimTx)
				simTxCondVar.wait(lock);
			canSimTx=false;

			pingSimTx=new PingMsgClass("ajaj", 222, 333, 0);
			std::string pingSimTxDeserStr;
			pingSimTx->SerStr(pingSimTxDeserStr);
			std::cout << "CloudReg Tx PING seq=" << pingSimTx->mSeqNr << std::endl;

			rbMqSend(connSimTx, RB_MQ_DIRECT_EXCHANGE, RB_MQ_BBREG_RES_KEY_UNIQUE_ID, pingSimTxDeserStr.c_str());

			delete pingSimTx;

			canSimRx=true;
			simRxCondVar.notify_one();
		}
	}
}
