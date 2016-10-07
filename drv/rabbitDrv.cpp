/*
 * rabbitDrv.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: rogal
 */

#include <iostream>

#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

#include "rabbitDrv.hpp"

void rbMQConsumerInit(void)
{
	char const hostname[]="192.168.7.1";
	char const exchange[]="amq.direct";
	char const routingkey[]="test";
	char const messagebody[]="messagebodyMR";
	const int port = 5672;
	int status;

	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;

	std::cout << __func__ << std::endl;


	conn = amqp_new_connection();

	std::cout << __func__ << " creating TCP socket" << std::endl;
	socket = amqp_tcp_socket_new(conn);
	if (!socket) {
		std::cout << "Error in amqp_tcp_socket_new" << std::endl;
	}

	std::cout << __func__ << " opening TCP socket" << std::endl;
	status = amqp_socket_open(socket, hostname, port);
	if (status) {
		std::cout << "Error in amqp_socket_open" << std::endl;
	}

	amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
	amqp_channel_open(conn, 1);
	amqp_get_rpc_reply(conn);

	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */
	amqp_basic_publish(conn,
			1,
			amqp_cstring_bytes(exchange),
			amqp_cstring_bytes(routingkey),
			0,
			0,
			&props,
			amqp_cstring_bytes(messagebody));

	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
	return;
}



