#ifndef __RESPONDER_H
#define __RESPONDER_H

#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>

#include "utils.h"

struct client_info
{
    int sd_client;
};

void *responder                     (void *info);
int   find_available_next_host_index(void);

#endif
