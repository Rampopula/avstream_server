#ifndef __UDPSOCK_H__
#define __UDPSOCK_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "common.h"


#define UDPSOCK_BUF_SIZE		(32768)
#define UDPSOCK_SERVER_PATH		"/tmp/avudpsock"
#define UDPSOCK_CLIENT_PATH		"/tmp/avudpsockcli"


int32_t udpsock_open(int32_t send_delay);

int32_t udpsock_close(void);

int32_t udpsock_send(uint8_t *data, int32_t size);


#endif /* __UDPSOCK_H__ */
