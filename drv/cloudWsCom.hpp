/*
 * cloudWsCom.hpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */

#ifndef DRV_CLOUDWSCOM_HPP_
#define DRV_CLOUDWSCOM_HPP_

#include <thread>

class cloudWsComClass
{
	unsigned int txPingSeqNr=0, txPingRegState=0;
	unsigned int rxPingSeqNr, rxPingRegState, rxPingTimeStamp;

	//receives all messages, service them, sends responses
	std::thread rxThread;
	void rxThreadFunc(void);

	//sends PINGs, checks status and so on
	std::thread pollThread;
	void pollThreadFunc(void);

	//wait
	void waitGridDelay(unsigned long ulInterval);

public:
	void init(void);
	void shutdown(void);
};

#endif /* DRV_CLOUDWSCOM_HPP_ */
