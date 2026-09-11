#ifndef PROCESS_H
#define PROCESS_H

typedef struct Process Process;
typedef enum { NEW, READY, RUNNING, WAITING, TERMINATED } ProcessState;

// Constructor
Process* process_create(const char* pid, int arrival, int burst, int io_freq, int io_duration);
void process_destroy(Process* p);

// Getters y Setters
const char* process_get_pid(Process* p);
int process_get_arrival(Process* p);
int process_get_burst(Process* p);
int process_get_remaining(Process* p);
ProcessState process_get_state(Process* p);
void process_set_state(Process* p, ProcessState state);
int process_get_current_queue(Process* p);
void process_set_current_queue(Process* p, int q_level);

// Métodos
void process_execute_cycle(Process* p, int current_time);
int process_is_finished(Process* p);
int process_needs_io(Process* p);
void process_start_io(Process* p);
void process_execute_io_cycle(Process* p);
int process_is_io_finished(Process* p);

// Métricas
int process_get_response_time(Process* p);
int process_get_turnaround_time(Process* p);
int process_get_waiting_time(Process* p);

#endif