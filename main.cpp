/*
 * main.cpp
 *
 *  Created on: Sep 21, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"
#include "configJson.hpp"

#include <fstream>
#include <iostream>

#include "version.hpp"
#include "exampleLib.hpp"
#include "rabbitDrv.hpp"
#include "bbRegistration.hpp"
#include "json.h"

ConfigClass *configGlob = nullptr;

int main(int argc, char *argv[]){

	configGlob = new ConfigClass(GLOB_CONFIG_FILE);
	std::string str;
	configGlob->SerStr(str);
	std::cout << str;

	rbMQConsumerInit();



	bbRegistrationClass bbRegistration;

	bbRegistration.init();



	//library usage example
	sum(2, 3);

	return 0;
}

