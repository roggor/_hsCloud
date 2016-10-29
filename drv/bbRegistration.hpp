/*
 * bbRegistration.hpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */

#ifndef DRV_BBREGISTRATION_HPP_
#define DRV_BBREGISTRATION_HPP_

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "ISerDeser.hpp"
#include "rabbitDrv.hpp"

class bbRegistrationClass
{
	unsigned int rxSeqNumber, rxRegState, rxTimeStamp;
	unsigned int txSeqNumber=0, txRegState=0;

	bool canTx;
	bool canRx;

	std::mutex txRxMutex;
	std::condition_variable txCondVar;
	std::condition_variable rxCondVar;

	std::thread regResRxThread;
	std::thread regReqTxThread;

	void regReqTxThreadFunc(void);
	void regResRxThreadFunc(void);

	amqp_connection_state_t connRx;
	amqp_connection_state_t connTx;

	amqp_bytes_t queuenameRx;

public:
	std::string bbRegString;
	void init(void);
	void shutdown(void);
};

#endif /* DRV_BBREGISTRATION_HPP_ */
