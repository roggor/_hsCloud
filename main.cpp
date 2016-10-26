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
#include <getopt.h>
#include <string>
#include <unistd.h>

#include "mainCloudSim.hpp"
#include "version.hpp"
#include "rabbitDrv.hpp"
#include "bbRegistration.hpp"
#include "cli.hpp"

//3rd party
#include "json.h" //sources/headres are included into project

ConfigClass *configGlob = nullptr;

int main(int argc, char *argv[]){

	int c;
	int do_verbose=0, do_server=0;
	std::string configFile=GLOB_CONFIG_FILE;

	static struct option long_options[] =
	{
		{"server_mode",   no_argument,        0,    's'  },
		{"verbose_debug", no_argument,        0,    'v'  },
		{"config_file",   required_argument,  0,    'c'  },
		{0, 0, 0, 0}
	};


	while ((c = getopt_long (argc, argv, "svc:", long_options, NULL)) != -1)
	{
		switch (c)
		{
		case 's':
			do_server=1;
			break;

		case 'v':
			do_verbose=1;
			break;

		case 'c':
			configFile=optarg;
			break;

	    case '?':
	    	//printf("Help\n");
	    	break;

	    case 0:     /* getopt_long() set a variable, just keep going */
	        break;

		default:
			printf("%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
			break;
		}
	}

	//read in configuration from config file
	configGlob = new ConfigClass(GLOB_CONFIG_FILE);

	if (do_server)
	{
		mainCloudSim();
		return 0;
	}

	InitCliThread();
//	rbMQConsumerInit();

	bbRegistrationClass bbRegistration;
	bbRegistration.init();

	while(1) {}

	bbRegistration.shutdown();

	return 0;
}

