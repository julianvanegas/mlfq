#ifndef CONSOLE_ADAPTER_H
#define CONSOLE_ADAPTER_H

#include "../application/scheduler.h"
#include <stdio.h>

void export_execution_trace_console(MLFQScheduler* scheduler, FILE* output);

#endif