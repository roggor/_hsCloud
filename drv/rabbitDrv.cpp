/*
 * rabbitDrv.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"

#include <iostream>
#include <unistd.h>

#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

#include "rabbitDrv.hpp"

void rbMQConsumerInit(void)
{
	char const serverName[]=RB_MQ_SERVER_NAME;
	char const exchange[]="amq.direct";
	char const routingkey[]="test";
	char const messagebody[]="messagebodyMR";
	const int port = 5672;
	int status;

	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;

	conn = amqp_new_connection();

	socket = amqp_tcp_socket_new(conn);
	if (!socket) {
		std::cout << "Error in amqp_tcp_socket_new" << std::endl;
	}

	status = amqp_socket_open(socket, serverName, port);
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

	while(1)
	{
		std::cout << "Publish" << std::endl;
		amqp_basic_publish(conn,
			1,
			amqp_cstring_bytes(exchange),
			amqp_cstring_bytes(routingkey),
			0,
			0,
			&props,
			amqp_cstring_bytes(messagebody));
		sleep(1);
	}

	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
	return;
}



