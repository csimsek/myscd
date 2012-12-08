#include "utils.h"

extern struct config g_config;
extern char *g_config_file;

int config_parser(struct config *c)
{
    dictionary *ini;
    char *host;

    char errors[1024] = "";

    ini = iniparser_load(g_config_file);
    
    if (ini == NULL)
    {
        fprintf(stderr, "Error parsing ini file\n");
        return -1;
    }
    
    if (iniparser_getstr(ini, "myscd:pid_file") == NULL)
        strcat(errors, "* pid_file\n");
    if (iniparser_getstr(ini, "myscd:bind_address") == NULL)
        strcat(errors, "* bind_address\n");
    if (iniparser_getstr(ini, "myscd:bind_port") == NULL)
        strcat(errors, "* bind_port\n");
    if (iniparser_getstr(ini, "myscd:hosts") == NULL)
        strcat(errors, "* hosts\n");
    if (iniparser_getstr(ini, "myscd:host_check_interval") == NULL)
        strcat(errors, "* host_check_interval\n");
    if (iniparser_getstr(ini, "myscd:host_check_interval") == NULL)
        strcat(errors, "* max_clients\n");
    if (iniparser_getstr(ini, "myscd:syslog_identity") == NULL)
        strcat(errors, "* syslog_identity\n");

    if (strlen(errors))
    {
        fprintf(stderr, "required config elements:\n\n%s", errors);
        exit(-1);
    }
    
    c->pid_file        = (char *) malloc(1 + strlen(iniparser_getstr(ini, "myscd:pid_file")));
    c->bind_address    = (char *) malloc(1 + strlen(iniparser_getstr(ini, "myscd:bind_address")));
    c->hosts           = (char *) malloc(1 + strlen(iniparser_getstr(ini, "myscd:hosts")));
    c->syslog_identity = (char *) malloc(1 + strlen(iniparser_getstr(ini, "myscd:syslog_identity")));
    
    strcpy(c->pid_file       , iniparser_getstr(ini, "myscd:pid_file"));
    strcpy(c->bind_address   , iniparser_getstr(ini, "myscd:bind_address"));
    strcpy(c->hosts          , iniparser_getstr(ini, "myscd:hosts"));
    strcpy(c->syslog_identity, iniparser_getstr(ini, "myscd:syslog_identity"));
    
    c->bind_port           = iniparser_getint(ini, "myscd:bind_port", 22000);
    c->host_check_interval = iniparser_getint(ini, "myscd:host_check_interval", 10);
    c->max_clients         = iniparser_getint(ini, "myscd:max_clinets", 10);

    host = strtok(c->hosts, ",");
    
    while (host != NULL)
    {
        char *section_name = (char *) malloc(1024);
        c->host_infos[c->host_count] = (struct host_info *) malloc(sizeof(struct host_info));
        
        /* host name */
        strcpy(section_name, host);
        strcat(section_name, ":");
        strcat(section_name, "host_name");
        c->host_infos[c->host_count]->host_name = (char *) malloc(1 + strlen(iniparser_getstr(ini, section_name)));
        strcpy(c->host_infos[c->host_count]->host_name, iniparser_getstr(ini, section_name));
        
        /* user name */
        strcpy(section_name, host);
        strcat(section_name, ":");
        strcat(section_name, "user_name");
        
        c->host_infos[c->host_count]->user_name = (char *) malloc(1 + strlen(iniparser_getstr(ini, section_name)));
        strcpy(c->host_infos[c->host_count]->user_name, iniparser_getstr(ini, section_name));

        /* password */
        strcpy(section_name, host);
        strcat(section_name, ":");
        strcat(section_name, "password");
        
        c->host_infos[c->host_count]->password = (char *) malloc(1 + strlen(iniparser_getstr(ini, section_name)));
        strcpy(c->host_infos[c->host_count]->password, iniparser_getstr(ini, section_name));

        /* port */
        strcpy(section_name, host);
        strcat(section_name, ":");
        strcat(section_name, "port");
        c->host_infos[c->host_count]->port = iniparser_getint(ini, section_name, 3306);

        host = strtok(NULL, ",");
        c->host_count++;
    }
    return 0;
}

void usage(void)
{
    printf("\n myscd usage\n\n");
    printf("    -c <config file>\n");
    printf("    -k [start | stop | restart]\n");
    printf("\n");
}


void free_config(struct config *c)
{
    dictionary *ini;
    char *host;
    free(c->pid_file);
    free(c->bind_address);
    ini = iniparser_load(g_config_file);
    
    host = strtok(c->hosts, ",");
    c->host_count = 0;
    
    while (host != NULL)
    {
        free(c->host_infos[c->host_count]->host_name);
        free(c->host_infos[c->host_count]->user_name);
        free(c->host_infos[c->host_count]->password);
        free(c->host_infos[c->host_count]);
        
        c->host_count++;
        host = strtok(NULL, ",");
    }
    free(c->hosts);
    c->host_count = 0;
}
        
char* itoa(int val, int base)
{
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}
