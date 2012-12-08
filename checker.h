#ifndef __CHECKER_H
#define __CHECKER_H

#include <stdio.h>
#include <mysql/mysql.h>
#include <time.h>
#include <pthread.h>

#include "utils.h"

void *checker(void *param);

#endif
