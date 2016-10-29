/*
 * rabbitDrv.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: rogal
 */
#include "globConfs.hpp"

#include <iostream>
#include <unistd.h>

#include "rabbitDrv.hpp"

amqp_connection_state_t rbMQConnect(int port, std::string serverName)
{
	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	int status;

	conn = amqp_new_connection();

	socket = amqp_tcp_socket_new(conn);
	if (!socket) {
		std::cout << "Error in amqp_tcp_socket_new" << std::endl;
	}

	status = amqp_socket_open(socket, serverName.c_str(), port);
	if (status) {
		std::cout << "Error in amqp_socket_open" << std::endl;
	}

	amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
	amqp_channel_open(conn, 1);
	amqp_get_rpc_reply(conn);

	return conn;
}

void rbMqSend (amqp_connection_state_t conn, std::string exchange, std::string bindingkey, std::string message)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */

	amqp_basic_publish(conn,
			1,
			amqp_cstring_bytes(exchange.c_str()),
			amqp_cstring_bytes(bindingkey.c_str()),
			0,
			0,
			&props,
			amqp_cstring_bytes(message.c_str()));
}

amqp_bytes_t rbMqDeclareQueue(amqp_connection_state_t conn, std::string queueNameCstr, std::string exchange, std::string bindingkey)
{
	amqp_bytes_t queuename;
	amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_cstring_bytes(queueNameCstr.c_str()), 0, 0, 0, 1,
			amqp_empty_table);

	queuename = amqp_bytes_malloc_dup(r->queue);
	if (queuename.bytes == NULL) {
		std::cout << "Out of memory while copying queue name" << std::endl;
		return amqp_empty_bytes;
	}

	amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange.c_str()), amqp_cstring_bytes(bindingkey.c_str()),
			amqp_empty_table);

	return queuename;
}

void rbMqBasicConsume(amqp_connection_state_t conn, amqp_bytes_t queuename)
{
	amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
	amqp_maybe_release_buffers(conn);
}

void rbMqReceive(amqp_connection_state_t conn, amqp_bytes_t queuename, amqp_envelope_t *envelope)
{
	amqp_rpc_reply_t res;

	res = amqp_consume_message(conn, envelope, NULL, 0);

	if (AMQP_RESPONSE_NORMAL != res.reply_type) {
		std::cout<<"BREAK"<<std::endl;
		return;
	}

	//		printf("Delivery %u, exchange %.*s routingkey %.*s\n",
	//				(unsigned) envelope->delivery_tag,
	//				(int) envelope->exchange.len, (char *) envelope->exchange.bytes,
	//				(int) envelope->routing_key.len, (char *) envelope->routing_key.bytes);

	if (envelope->message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
		//			printf("Content-type: %.*s\n",
		//					(int) envelope->message.properties.content_type.len,
		//					(char *) envelope->message.properties.content_type.bytes);
	}
	//		printf("----\n");
}

void rbMqDisconnect(amqp_connection_state_t conn)
{
	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
}



