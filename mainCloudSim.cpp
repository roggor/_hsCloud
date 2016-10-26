/*
 * mainCloudSim.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"

#include <iostream>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "mainCloudSim.hpp"

void mainCloudSim(void)
{
	char const serverName[]=RB_MQ_SERVER_NAME;
	char const exchange[]="amq.direct";
	char const bindingkey[]="test";
	char const messagebody[]="messagebodyMR";
	const int port = 5672;
	int status;

	amqp_bytes_t queuename;

	std::cout << __func__ << "Server: " << RB_MQ_SERVER_NAME << std::endl;

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

	amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1,
	                                 amqp_empty_table);

	queuename = amqp_bytes_malloc_dup(r->queue);
	if (queuename.bytes == NULL) {
		std::cout << "Out of memory while copying queue name" << std::endl;
		return;
	}

	amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey),
			amqp_empty_table);

	amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);

	for (;;) {
		amqp_rpc_reply_t res;
		amqp_envelope_t envelope;

		amqp_maybe_release_buffers(conn);

		res = amqp_consume_message(conn, &envelope, NULL, 0);

		if (AMQP_RESPONSE_NORMAL != res.reply_type) {
	    	  std::cout<<"BREAK"<<std::endl;
			break;
		}

//		printf("Delivery %u, exchange %.*s routingkey %.*s\n",
//				(unsigned) envelope.delivery_tag,
//				(int) envelope.exchange.len, (char *) envelope.exchange.bytes,
//				(int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);

		if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
//			printf("Content-type: %.*s\n",
//					(int) envelope.message.properties.content_type.len,
//					(char *) envelope.message.properties.content_type.bytes);
		}
//		printf("----\n");

		char* ptr=static_cast<char*>(envelope.message.body.bytes);

		for (int k=0; k<envelope.message.body.len; k++)
			std::cout << (ptr[k]);
		std::cout << std::endl;
		amqp_destroy_envelope(&envelope);
	}

	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
}
