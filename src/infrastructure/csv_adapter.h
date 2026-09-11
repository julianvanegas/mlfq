#ifndef CSV_ADAPTER_H
#define CSV_ADAPTER_H
#include "../domain/queue.h"

void export_results_csv(const char* filename, Queue* terminated_processes);

#endif