/*
 * ISerDeser.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef DRV_ISERDESER_HPP_
#define DRV_ISERDESER_HPP_

#include <string>

#include "json.h"

class ISerDeser
{
public:
   virtual ~ISerDeser( void ) {};
   virtual void Serialize( Json::Value& root ) =0;
   virtual void Deserialize( Json::Value& root) =0;

   void SerStr ( std::string& str );
   bool DeserStr ( std::string& str );
};

#endif /* DRV_ISERDESER_HPP_ */
