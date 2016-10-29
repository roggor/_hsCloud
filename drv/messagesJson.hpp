/*
 * messagesJson.hpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */

#ifndef DRV_MESSAGESJSON_HPP_
#define DRV_MESSAGESJSON_HPP_

#include "ISerDeser.hpp"

class MsgClass : public ISerDeser
{
protected:
	std::string mMessageType;
	std::string mReplyToQueueName;
public:
	MsgClass(std::string mMessageType, std::string mReplyToQueueName)
	: mMessageType(mMessageType), mReplyToQueueName(mReplyToQueueName) {}
	virtual ~MsgClass(void) {}

	void Serialize( Json::Value& root ) override {}
	void Deserialize( Json::Value& root) override {}
};

class PingMsgClass : public MsgClass
{
public:
	PingMsgClass( std::string mReplyToQueueName, int mSeqNr, int mRegState, int mTimeStamp)
		: MsgClass("regPing", mReplyToQueueName), mSeqNr(mSeqNr), mRegState(mRegState), mTimeStamp(mTimeStamp) {}
	virtual ~PingMsgClass( void ){}

	void Serialize( Json::Value& root ) override;
	void Deserialize( Json::Value& root) override;

public: //should be private
	int mSeqNr;
	int mRegState;
	int mTimeStamp;
};

#endif /* DRV_MESSAGESJSON_HPP_ */
