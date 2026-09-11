#include "console_adapter.h"

void export_execution_trace_console(MLFQScheduler* scheduler, FILE* output) {
    if (!output) return;

    fprintf(output, "\nTraza de ejecucion\n");
    fprintf(output, "%-6s %-8s %-8s %-8s %-20s\n", "Time", "Q0", "Q1", "Q2", "QIO");

    for (int i = 0; i < scheduler_get_trace_size(scheduler); i++) {
        const SchedulerTraceEntry* entry = scheduler_get_trace_entry(scheduler, i);
        fprintf(output, "%-6d %-8s %-8s %-8s %-20s\n",
            entry->time, entry->q0, entry->q1, entry->q2, entry->qio);
    }
}