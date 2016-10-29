/*
 * configJson.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef DRV_CONFIGJSON_HPP_
#define DRV_CONFIGJSON_HPP_

#include <iostream>

#include "ISerDeser.hpp"

class ConfigClass : public ISerDeser
{
	std::string    mConfigName;
	std::string    mConfigVersion;
	std::string    mWashCarAddress;
	std::string    mBbId;
	int            mWashId;

public:
	ConfigClass( void );
	ConfigClass( std::string filePath);
	virtual ~ConfigClass( void ){}

	virtual void Serialize( Json::Value& root );
	virtual void Deserialize( Json::Value& root);

	std::string getBbId(void) {return mBbId;}
};

#endif /* DRV_CONFIGJSON_HPP_ */
