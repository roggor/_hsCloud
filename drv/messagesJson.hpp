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
public:
	MsgClass(std::string mMessageType)
	: mMessageType(mMessageType) {}
	virtual ~MsgClass(void) {}

	void Serialize( Json::Value& root ) override {}
	void Deserialize( Json::Value& root) override {}
};

class PingMsgClass : public MsgClass
{
public:
	PingMsgClass( int mSeqNr, int mRegState, int mTimeStamp)
		: MsgClass("regPing"), mSeqNr(mSeqNr), mRegState(mRegState), mTimeStamp(mTimeStamp) {}
	virtual ~PingMsgClass( void ){}

	void Serialize( Json::Value& root ) override;
	void Deserialize( Json::Value& root) override;

public: //should be private
	int mSeqNr;
	int mRegState;
	int mTimeStamp;
};


/*************************************************/
class PingMinMsgClass : public MsgClass
{
public:
	PingMinMsgClass(void)
		: MsgClass("regPingMin") {}
	virtual ~PingMinMsgClass( void ){}

	void Serialize( Json::Value& root ) override;
	void Deserialize( Json::Value& root) override;
};
/*************************************************/











#endif /* DRV_MESSAGESJSON_HPP_ */
