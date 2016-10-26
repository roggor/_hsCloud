/*
 * configJson.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef DRV_CONFIGJSON_HPP_
#define DRV_CONFIGJSON_HPP_

#include <iostream>

#include "../libs/serDeser/ISerDeser.hpp"

class ConfigClass : public ISerDeser
{
public:
	ConfigClass( void );
	ConfigClass( std::string filePath);
	virtual ~ConfigClass( void ){};

	virtual void Serialize( Json::Value& root );
	virtual void Deserialize( Json::Value& root);

private:
	std::string    mConfigName;
	std::string    mConfigVersion;
	std::string    mWashCarAddress;
	int            mWashId;
};

#endif /* DRV_CONFIGJSON_HPP_ */
