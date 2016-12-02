/*
 * globConfs.hpp
 *
 *  Created on: Oct 6, 2016
 *      Author: rogal
 */

#ifndef GLOBCONFS_HPP_
#define GLOBCONFS_HPP_

#define GLOB_CONFIG_FILE "../config/config.json"

#include "configJson.hpp"
extern ConfigClass *configGlob;

/* WS related config */
#define WS_SERVER_IP "149.202.33.120"
#define WS_SERVER_PORT 8143
#define WS_SERVER_URI "/carwash-core/carwash/bb_0x0001/communication"
#define WS_ETH_IF "eth0"


#define PROTO_R_DEV_NAME "/dev/ttyS1"

#endif /* GLOBCONFS_HPP_ */
