/*
 * globConfs.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef GLOBCONFS_HPP_
#define GLOBCONFS_HPP_

#define GLOB_CONFIG_FILE "../config/config.json"


//Rabbit MQ related configs
#ifdef TARGET_ARM
	#define RB_MQ_SERVER_NAME "192.168.7.1"
#else
	#define RB_MQ_SERVER_NAME "localhost"
#endif

#endif /* GLOBCONFS_HPP_ */
