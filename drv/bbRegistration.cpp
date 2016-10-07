/*
 * bbRegistration.cpp
 *
 *  Created on: Oct 1, 2016
 *      Author: rogal
 */
#include "bbRegistration.hpp"

#include <iostream>
#include <thread>


void call_from_thread(void)
{
	std::cout << "Hello from thread" << std::endl;
}

void bbRegistrationClass::init(void)
{
	std::cout << "Init";

	//Launch a thread
	std::thread t1(call_from_thread);
}


