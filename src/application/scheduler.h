#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "../domain/process.h"
#include "../domain/queue.h"

typedef struct MLFQScheduler MLFQScheduler;

typedef struct {
	int time;
	char q0[16];
	char q1[16];
	char q2[16];
	char qio[256];
} SchedulerTraceEntry;

MLFQScheduler* scheduler_create(int boost_interval);
void scheduler_destroy(MLFQScheduler* sched);
void scheduler_add_process(MLFQScheduler* sched, Process* p);
void scheduler_tick(MLFQScheduler* sched);
int scheduler_is_done(MLFQScheduler* sched);
Queue* scheduler_get_terminated(MLFQScheduler* sched);
int scheduler_get_current_time(MLFQScheduler* sched);
int scheduler_get_trace_size(MLFQScheduler* sched);
const SchedulerTraceEntry* scheduler_get_trace_entry(MLFQScheduler* sched, int index);

#endif