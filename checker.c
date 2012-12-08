#include "checker.h"

extern pthread_mutex_t responder_mutex;
extern struct config g_config;
extern int g_run_checker;

void *checker(void *param)
{
    
    int i, available_count;
    while (g_run_checker)
    {
        available_count = 0;
        for (i = 0; i < g_config.host_count; i++)
        {
            pthread_mutex_lock(&responder_mutex);
            do
            {
                MYSQL *my = mysql_init(NULL);
                g_config.host_infos[i]->status = FALSE;
                if (my == NULL)
                {
                    syslog(LOG_ERR, "mysql_init(): %s", mysql_error(my));
                    break;
                }
                
                if (!mysql_real_connect(my,
                                        g_config.host_infos[i]->host_name,
                                        g_config.host_infos[i]->user_name,
                                        g_config.host_infos[i]->password,
                                        "",
                                        g_config.host_infos[i]->port,
                                        NULL, 0))
                {
                    syslog(LOG_ERR,
                           "mysql_real_connect(): %s - %s",
                           g_config.host_infos[i]->host_name,
                           mysql_error(my));
                    break;
                }
                g_config.host_infos[i]->status = TRUE;
                mysql_close(my);
                available_count++;
                syslog(LOG_INFO, "checker is up: %ld", time(NULL));
            }
            while (0);
            pthread_mutex_unlock(&responder_mutex);
        }
        g_config.available_count = available_count;
        sleep(g_config.host_check_interval);
    }
    return NULL;
}
