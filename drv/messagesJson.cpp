/*
 * messagesJson.cpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"
#include "messagesJson.hpp"

#include <iostream>
#include <unistd.h>

void PingMsgClass::Serialize( Json::Value& root )
{
	root["mMessageType"] = mMessageType;
	root["mSeqNr"] = mSeqNr;
	root["mRegState"] = mRegState;
	root["mTimeStamp"] = mTimeStamp;
	root["mReplyToQueueName"] = mReplyToQueueName;
}

void PingMsgClass::Deserialize( Json::Value& root )
{
	mMessageType = root.get("mMessageType","").asString();
	mSeqNr = root.get("mSeqNr",0).asInt();
	mRegState = root.get("mRegState",0).asInt();
	mTimeStamp = root.get("mTimeStamp",0).asInt();
	mReplyToQueueName = root.get("mReplyToQueueName","").asString();
}
