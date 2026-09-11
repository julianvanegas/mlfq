#include "process.h"
#include <stdlib.h>
#include <string.h>

struct Process {
    char pid[10];
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int finish_time;
    int first_response_time;
    int current_queue;
    ProcessState state;
    int io_frequency;
    int io_duration;
    int current_cpu_burst_used;
    int current_io_time_remaining;
};

// Constructor y Destructor
Process* process_create(const char* pid, int arrival, int burst, int io_freq, int io_duration) {
    Process* p = (Process*)malloc(sizeof(Process));
    if (!p) return NULL;

    strncpy(p->pid, pid, sizeof(p->pid) - 1);
    p->pid[sizeof(p->pid) - 1] = '\0';
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->remaining_time = burst;
    p->start_time = -1;
    p->finish_time = -1;
    p->first_response_time = -1;
    p->current_queue = 0; 
    p->state = NEW;
    p->io_frequency = io_freq;
    p->io_duration = io_duration;
    p->current_cpu_burst_used = 0;
    p->current_io_time_remaining = 0;

    return p;
}

void process_destroy(Process* p) {
    if (p) free(p);
}

// Getters y Setters
const char* process_get_pid(Process* p) {
    return p->pid;
}
int process_get_arrival(Process* p) {
    return p->arrival_time;
}
int process_get_burst(Process* p) {
    return p->burst_time;
}
int process_get_remaining(Process* p) {
    return p->remaining_time;
}
ProcessState process_get_state(Process* p) {
    return p->state;
}
void process_set_state(Process* p, ProcessState state) {
    p->state = state;
}
int process_get_current_queue(Process* p) {
    return p->current_queue;
}
void process_set_current_queue(Process* p, int q_level) {
    p->current_queue = q_level;
}

// Funciones
void process_execute_cycle(Process* p, int current_time) {
    if (p->first_response_time == -1) {
        p->first_response_time = current_time;
        p->start_time = current_time;
    }
    p->remaining_time--;
    p->current_cpu_burst_used++;
    
    if (p->remaining_time == 0) {
        p->finish_time = current_time + 1;
        p->state = TERMINATED;
    }
}

int process_is_finished(Process* p) {
    return p->remaining_time == 0;
}

int process_needs_io(Process* p) {
    return (p->io_frequency > 0 && p->current_cpu_burst_used >= p->io_frequency && p->remaining_time > 0);
}

void process_start_io(Process* p) {
    p->state = WAITING;
    p->current_io_time_remaining = p->io_duration;
    p->current_cpu_burst_used = 0; 
}

void process_execute_io_cycle(Process* p) {
    if (p->current_io_time_remaining > 0) p->current_io_time_remaining--;
}

int process_is_io_finished(Process* p) {
    return p->current_io_time_remaining <= 0;
}

// Métricas
int process_get_response_time(Process* p) {
    return p->first_response_time - p->arrival_time;
}

int process_get_turnaround_time(Process* p) {
    return p->finish_time - p->arrival_time;
}

int process_get_waiting_time(Process* p) {
    return process_get_turnaround_time(p) - p->burst_time;
}