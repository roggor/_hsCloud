/*
 * rabbitDrv.hpp
 *
 *  Created on: Sep 22, 2016
 *      Author: rogal
 */

#ifndef DRV_RABBITDRV_HPP_
#define DRV_RABBITDRV_HPP_

#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

amqp_connection_state_t rbMQConnect(int port, std::string serverName);
amqp_bytes_t rbMqDeclareQueue(amqp_connection_state_t conn, std::string queueNameCstr, std::string exchange, std::string bindingkey);
void rbMqBasicConsume(amqp_connection_state_t conn, amqp_bytes_t queuename);
void rbMqReceive(amqp_connection_state_t conn, amqp_bytes_t queuename, amqp_envelope_t *envelope);
void rbMqSend (amqp_connection_state_t conn, std::string exchange, std::string bindingkey, std::string message);
void rbMqDisconnect(amqp_connection_state_t conn);

#endif /* DRV_RABBITDRV_HPP_ */
