/*
 * configJson.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */
#include "configJson.hpp"

#include <fstream>


void ConfigClass::Serialize( Json::Value& root )
{
	root["mConfigName"] = mConfigName;
	root["mConfigVersion"] = mConfigVersion;
	root["mWashCarAddress"] = mWashCarAddress;
	root["mBbId"] = mBbId;
	root["mWashId"] = mWashId;
}

void ConfigClass::Deserialize( Json::Value& root )
{
	mConfigName = root.get("mConfigName","").asString();
	mConfigVersion = root.get("mConfigVersion","").asString();
	mWashCarAddress = root.get("mWashCarAddress","").asString();
	mBbId = root.get("mBbId","").asString();
	mWashId = root.get("mWashId",0).asInt();
}

ConfigClass::ConfigClass( std::string filePath)
{
	std::ifstream file(filePath);
	std::stringstream strStream;
	strStream << file.rdbuf();//read the file
	std::string str = strStream.str();//str holds the content of the file

	DeserStr(str);
}
