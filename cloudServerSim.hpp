/*
 * cloudServerSim.hpp
 *
 *  Created on: Oct 18, 2016
 *      Author: rogal
 */

#ifndef CLOUDSERVERSIM_HPP_
#define CLOUDSERVERSIM_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>

#include "rabbitDrv.hpp"

class cloudServerSimClass
{


	bool canSimTx;
	bool canSimRx;

	std::mutex simTxRxMutex;
	std::condition_variable simTxCondVar;
	std::condition_variable simRxCondVar;

	std::thread simRxThread;
	std::thread simTxThread;

	void simTxThreadFunc(void);
	void simRxThreadFunc(void);

	amqp_connection_state_t connSimRx;
	amqp_connection_state_t connSimTx;

	amqp_bytes_t queuenameSimRx;

public:
	void init(void);
	void shutdown(void);
};

#endif /* CLOUDSERVERSIM_HPP_ */
