/*
 * wsApi.hpp
 *
 *  Created on: Nov 8, 2016
 *      Author: rogal
 */

#ifndef DRV_LIBWSAPI_HPP_
#define DRV_LIBWSAPI_HPP_

#include "libwebsockets.h"

#define MAX_TX_PAYLOAD 1400
#define MAX_RX_PAYLOAD 1400

void libwsApi_init(void); //creates thread
int libwsApi_send(char *pData, int dataLen);
int libwsApi_recv(char *pData);
void libwsApi_shutdown(void);

#endif /* DRV_LIBWSAPI_HPP_ */
