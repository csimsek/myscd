#include "responder.h"

extern pthread_mutex_t responder_mutex;

extern struct config g_config;

static int current_host_index = 0;

void *responder(void *info)
{
    char buffer[1024], output[1024];
    struct client_info *ci = (struct client_info *) info;
    int sd_client = ci->sd_client;
    
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(output, 0, sizeof(output));

        if (recv(sd_client, buffer, sizeof(buffer), 0) == -1)
            break;

        if (strncmp("next", buffer, 4) == 0)
        {
            if (!g_config.available_count)
            {
                syslog(LOG_ERR, "no_host_available(1)");
                strcpy(output, "error:no_host_available(1)\r\n");
            }
            else
            {
                int next_host;
                strcpy(output, "next_mysql_server:");
            
                pthread_mutex_lock(&responder_mutex);

                if ((next_host = find_available_next_host_index()) == -1)
                {
                    syslog(LOG_ERR, "no_host_available(2)");
                    strcpy(output, "error:no_host_available(2)\r\n");
                }
                else
                {
                    strcat(output, g_config.host_infos[next_host]->host_name);
                    strcat(output, ",");
                    strcat(output, g_config.host_infos[next_host]->user_name);
                    strcat(output, ",");
                    strcat(output, g_config.host_infos[next_host]->password);
                    strcat(output, ",");
                    strcat(output, itoa(g_config.host_infos[next_host]->port, 10));
                    strcat(output, "\r\n");
                }
                pthread_mutex_unlock(&responder_mutex);
            }
        }
            
        else if (strncmp(buffer, "quit", 4) == 0)
            break;
        else
            strcpy(output, "error:unknown_command\r\n");
        
        if (send(sd_client, output, strlen(output), 0) == -1)
            break;
    }
    close(sd_client);
    return NULL;
}

int find_available_next_host_index(void)
{
    int host_check_count = 0;
    current_host_index++;
    if (current_host_index >= g_config.host_count)
        current_host_index = 0;
    while (!g_config.host_infos[current_host_index]->status)
    {
        syslog(LOG_INFO, "finding %ld", time(NULL));
        current_host_index++;
        host_check_count++;
        if (host_check_count >= g_config.host_count)
            return -1;
        if (current_host_index >= g_config.host_count)
            current_host_index = 0;
    }
    return current_host_index;
}
