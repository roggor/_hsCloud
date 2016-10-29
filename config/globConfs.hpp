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
//#ifdef TARGET_ARM
	#define RB_MQ_SERVER_NAME "192.168.7.1"
//#else
//	#define RB_MQ_SERVER_NAME "localhost"
//#endif

#define RB_MQ_PORT 5672

//global ones, for each BB the same
#define RB_MQ_DIRECT_EXCHANGE   "amq.direct"

#define RB_MQ_BBREG_REQ_QUEUE  "regReqQ"
#define RB_MQ_BBREG_REQ_KEY    "regReqK"


#include "configJson.hpp"
extern ConfigClass *configGlob;

//must be appended with BB_ID, unique for each BB
#define RB_MQ_BBREG_RES_QUEUE           "regResQ_"
#define RB_MQ_BBREG_RES_QUEUE_UNIQUE_ID (RB_MQ_BBREG_RES_QUEUE+configGlob->getBbId())
#define RB_MQ_BBREG_RES_KEY             "regResK_"
#define RB_MQ_BBREG_RES_KEY_UNIQUE_ID   (RB_MQ_BBREG_RES_KEY+configGlob->getBbId())

#endif /* GLOBCONFS_HPP_ */
