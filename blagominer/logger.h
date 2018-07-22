#pragma once
#include "blagominer.h"
#include "inout.h"
#include "error.h"


//logger variables
extern bool use_log;
extern FILE * fp_Log;

//logger functions
void Log_init(void);
void Log(char const *const strLog);
void Log_llu(unsigned long long const llu_num);
void Log_u(size_t const u_num);
void Log_server(char const *const strLog);
