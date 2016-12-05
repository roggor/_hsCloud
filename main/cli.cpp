/*
 * cli.cpp
 *
 *  Created on: Oct 10, 2016
 *      Author: rogal
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>


#include "protocolR.hpp"
#include "cli.hpp"

/*
 * extern C to dla kompilatora g++, bo poniewaz w g++ moga byc przeciazone funkcje
 * tzn w zaleznosci od parametrow ta funkcja bedzie sie troche inaczej nazywac w .o (objdump)
 * i wtedy nie jest znajdowana w bibliotece
 */
extern "C"
{
#include <libcli.h>
}

static pthread_t cli_thread;

int usartComRxPrintStats(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	//TODO: change it somehow
	char buf[1000];
	unsigned int ret;
	ret=protoGetGlobalStats(buf);
    cli_print(cli, (char*)"%s\nSize: %u out of %lu\n", buf, ret, sizeof(buf));
    return CLI_OK;
}

int usartComRxResetStats(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	//TODO: change it somehow
	char buf[100];
	unsigned int ret;

	ret=protoResetGlobalStats(buf);
	cli_print(cli, (char*)"%s\nSize: %u out of %lu\n", buf, ret, sizeof(buf));
    return CLI_OK;
}

int stanSetDisp(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	cli_print(cli, (char*)"stanSetDisp\n");
	return CLI_OK;
}

int stanStats(struct cli_def *cli, char *command, char *argv[], int argc)
{
	cli_print(cli, (char*)"stanStats\n");
	return CLI_OK;
}

void *InitCli(void *arg)
{
	struct sockaddr_in servaddr;
	struct cli_command *stats, *stan;
	struct cli_def *cli;
	int on = 1, x, s;

	cli = cli_init();

	// Set the hostname (shown in the the prompt)
	cli_set_hostname(cli, (char*)"hsCloud");
	// Set the greeting
	cli_set_banner(cli, (char*)"Welcome to hsCloud BB.");

	// Set up a few simple one-level commands
	stats=cli_register_command(cli, NULL, (char*)"prot", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (char*)"help prot");
	cli_register_command(cli, stats, (const char*)"showStats", usartComRxPrintStats, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (const char*)"help showStats");
	cli_register_command(cli, stats, (const char*)"resetStats", usartComRxResetStats, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (const char*)"help resetStats");
//	cli_register_command(cli, stats, (char*)"stanStats", stanStats, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (char*)"help stanStats");

	stan=cli_register_command(cli, NULL, (char*)"stan", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (char*)"help stan");
	cli_register_command(cli, stan, (const char*)"setDisp", stanSetDisp, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, (const char*)"help stanSetDisp");
	// Create a socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	// Listen on port 12345
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(12345);
	bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr));

	// Wait for a connection
	listen(s, 2);

	while ((x = accept(s, NULL, 0)))
	{
		// Pass the connection off to libcli
		cli_loop(cli, x);
		close(x);
	}

	// Free data structures
	cli_done(cli);
}

void InitCliThread(void)
{
	int res;
	res = pthread_create(&cli_thread, NULL, InitCli, NULL);
	if (res != 0) {
		perror("Thread cli_thread creation failed");
		exit(EXIT_FAILURE);
	}
}

void ShutdownCliThread(void)
{
	pthread_join(cli_thread, NULL);
}
