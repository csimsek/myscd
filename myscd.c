#include "myscd.h"

struct config g_config;
char *g_config_file = NULL;

extern char *optarg;

int main(int argc, char *argv[])
{
    char *command = NULL;
    char optchar;
    int status;
    
    while ((optchar = getopt(argc, argv, "c:k:")) != -1)
    {
        switch (optchar)
        {
        case 'c':
            g_config_file = strdup(optarg);
            break;
        case 'k':
            command = strdup(optarg);
            break;
        default:
            usage();
            return 0;
        }
    }
    openlog(g_config_file, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (g_config_file == NULL || command == NULL)
    {
        usage();
        return 0;
    }

    config_parser(&g_config);
    
    openlog(g_config.syslog_identity, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    
    status = check_daemon();

    if (strncmp("start", command, 5) == 0)
    {
        if (!status)
        {
            syslog(LOG_NOTICE, "start_daemon()");
            start_daemon();
        }
        else
            fprintf(stderr, "already running\n");
    }
    else if (strncmp("stop", command, 4) == 0)
    {
        if (status)
        {
            syslog(LOG_NOTICE, "stop_daemon()");
            stop_daemon();
        }
        else
            fprintf(stderr, "not running\n");
    }
    else if (strncmp("restart", command, 7) == 0)
    {
        if (status)
        {
            syslog(LOG_NOTICE, "restart");
            stop_daemon();
            sleep(1);
            start_daemon();
        }
        else
        {
            fprintf(stderr, "not running\ntrying to start...\n");
            syslog(LOG_NOTICE, "start_daemon()");
            start_daemon();
        }
    }
    else
    {
        fprintf(stderr, "unknown command\n");
        usage();
    }
    return 0;
}
