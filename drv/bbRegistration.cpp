/*
 * bbRegistration.cpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */
#include "bbRegistration.hpp"

#include <iostream>
#include <unistd.h>

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

const std::string bbRegistrationClass::rxQueueName="";
const std::string bbRegistrationClass::txQueueName="kolejkaReplyTo";

void bbRegistrationClass::regResRxThreadFunc(void)
{
	while(1)
	{
		{
			std::unique_lock<std::mutex> lock(txRxMutex);
			while(!canRx)
				rxCondVar.wait(lock);
			canRx=false;

			std::cout << "Hello from thread regResRxThreadFunc" << std::endl;

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

			pingTx=new PingMsgClass(txSeqNumber, txRegState, 0, txQueueName);

			std::string pingTxDeserStr;
			pingTx->SerStr(pingTxDeserStr);
			std::cout << "Transmitting:" << std::endl << pingTxDeserStr << std::endl;

			canRx=true;
			rxCondVar.notify_one();
		}
		txSeqNumber++;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void bbRegistrationClass::init(void)
{
	canTx=true;
	canRx=false;

	//Lunch the thread to rx responses from Cloud
	regResRxThread=std::thread(&bbRegistrationClass::regResRxThreadFunc, this);

	//Lunch the thread to tx requests to Cloud
    regReqTxThread=std::thread(&bbRegistrationClass::regReqTxThreadFunc, this);
}

void bbRegistrationClass::shutdown(void)
{
	regResRxThread.join();
	regReqTxThread.join();
}



void PingMsgClass::Serialize( Json::Value& root )
{
	root["mSeqNr"] = mSeqNr;
	root["mRegState"] = mRegState;
	root["mTimeStamp"] = mTimeStamp;
	root["mReplyToQueueName"] = mReplyToQueueName;
}

void PingMsgClass::Deserialize( Json::Value& root )
{
	mSeqNr = root.get("mSeqNr",0).asInt();
	mRegState = root.get("mRegState",0).asInt();
	mTimeStamp = root.get("mTimeStamp",0).asInt();
	mReplyToQueueName = root.get("mReplyToQueueName","").asString();
}



