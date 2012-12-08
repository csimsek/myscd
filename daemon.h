#ifndef __DAEMON_H
#define __DAEMON_H

#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "utils.h"
#include "checker.h"
#include "responder.h"

void  start_daemon        (void);
void  stop_daemon         (void);
int   check_daemon        (void);
int   lock_pid_file       (void);
void  unlock_pid_file     (void);
void  check_create_pid    (void);
int   read_pid_file       (void);
void  signal_chld_handler (int s);
void  signal_hup_handler  (int s);
void  signal_term_handler (int s);
void  start_checker       (void);
void  stop_checker        (void);

#endif
