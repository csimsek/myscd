#ifndef __UTILS_H
#define __UTILS_H

#include <time.h>
#include <syslog.h>

#include "iniparser/iniparser.h"

#define L_ERROR        0
#define L_WARNING      1
#define L_NOTICE       2

#ifndef FALSE
#define	FALSE	0
#endif
#ifndef TRUE
#define	TRUE	1
#endif

#define MAX_HOST_COUNT 10

struct host_info
{
    char *host_name,
        *user_name,
        *password;
    int port,
        status;
};

struct config
{
    char *pid_file,
        *bind_address,
        *hosts,
        *syslog_identity;
    int bind_port,
        host_check_interval,
        host_count,
        max_clients,
        available_count,
        grace_period;
    struct host_info *host_infos[MAX_HOST_COUNT];
};

int   config_parser (struct config *p);
void free_config    (struct config *p);
void  usage         (void);
char* itoa          (int val, int base);

#endif
