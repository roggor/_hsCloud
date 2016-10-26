/*
 * bbRegistration.hpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */

#ifndef DRV_BBREGISTRATION_HPP_
#define DRV_BBREGISTRATION_HPP_

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../libs/serDeser/ISerDeser.hpp"

class bbRegistrationClass
{
	unsigned int rxSeqNumber, rxRegState, rxTimeStamp;
	static const std::string rxQueueName;

	unsigned int txSeqNumber, txRegState;
	static const std::string txQueueName;

	bool canTx;
	bool canRx;

	std::mutex txRxMutex;
	std::condition_variable txCondVar;
	std::condition_variable rxCondVar;

	std::thread regResRxThread;
	std::thread regReqTxThread;

	void regReqTxThreadFunc(void);
	void regResRxThreadFunc(void);
public:
	std::string bbRegString;
	void init(void);
	void shutdown(void);
};

class PingMsgClass : public ISerDeser
{
public:
	PingMsgClass( int mSeqNr, int mRegState, int mTimeStamp, std::string mReplyToQueueName)
		: mSeqNr(mSeqNr), mRegState(mRegState), mTimeStamp(mTimeStamp), mReplyToQueueName(mReplyToQueueName)
		{}
	virtual ~PingMsgClass( void ){};

	virtual void Serialize( Json::Value& root );
	virtual void Deserialize( Json::Value& root);

private:
	int mSeqNr;
	int mRegState;
	int mTimeStamp;
	std::string mReplyToQueueName;
};

#endif /* DRV_BBREGISTRATION_HPP_ */
