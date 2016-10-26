/*
 * ISerDeser.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef LIBS_SERDESER_ISERDESER_HPP_
#define LIBS_SERDESER_ISERDESER_HPP_

#include <string>

#include "json.h"

class ISerDeser
{
public:
   virtual ~ISerDeser( void ) {};
   virtual void Serialize( Json::Value& root ) =0;
   virtual void Deserialize( Json::Value& root) =0;

   //serializes fields to JSON node, and then to nice string
   void SerStr ( std::string& str );

   //deserializes string to JSON and then to fields
   bool DeserStr ( std::string& str );
};

#endif /* LIBS_SERDESER_ISERDESER_HPP_ */
