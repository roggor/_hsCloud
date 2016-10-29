/*
 * ISerDeser.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */
#include "../libs/serDeser/ISerDeser.hpp"

void ISerDeser::SerStr(std::string& str)
{
	Json::Value serializeRoot;
	this->Serialize(serializeRoot);

	Json::StyledWriter writer;
	str = writer.write( serializeRoot );
}

bool ISerDeser::DeserStr(std::string& str)
{
	Json::Value deserializeRoot;
	Json::Reader reader;

	if ( !reader.parse(str, deserializeRoot) )
		return false;

	this->Deserialize(deserializeRoot);
	return true;
}
