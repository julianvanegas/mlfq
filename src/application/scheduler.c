#include "scheduler.h"
#include "io_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct MLFQScheduler {
    Queue* q0; // Prioridad alta, quantum = 1 ciclos
    Queue* q1; // Prioridad media, quantum = 2 ciclos
    Queue* q2; // Prioridad baja, quantum = 4 ciclos
    Queue* terminated;
    IOManager* io_mgr;
    Process* current_running;
    int current_time;
    int current_quantum_used;
    int current_queue_level;
    int boost_interval;
    SchedulerTraceEntry* trace;
    int trace_size;
    int trace_capacity;
};

// Constructor y Destructor
MLFQScheduler* scheduler_create(int boost_interval) {
    MLFQScheduler* s = (MLFQScheduler*)malloc(sizeof(MLFQScheduler));
    if (!s) return NULL;
    
    s->q0 = queue_create();
    s->q1 = queue_create();
    s->q2 = queue_create();
    s->terminated = queue_create();
    s->io_mgr = io_manager_create();
    s->current_time = 0;
    s->current_running = NULL;
    s->current_quantum_used = 0;
    s->current_queue_level = 0;
    s->boost_interval = boost_interval; // Configura la frecuencia del Priority Boost
    s->trace = NULL;
    s->trace_size = 0;
    s->trace_capacity = 0;
    return s;
}

void scheduler_destroy(MLFQScheduler* s) {
    if (!s) return;
    queue_destroy(s->q0);
    queue_destroy(s->q1);
    queue_destroy(s->q2);
    queue_destroy(s->terminated);
    io_manager_destroy(s->io_mgr);
    free(s->trace);
    free(s);
}

// Arreglo con los quantums definidos para cada cola de prioridad
static const int QUANTUMS[3] = {1, 2, 4};

// Función auxiliar privada para mover procesos entre colas para el Priority Boost.
static void move_queue(Queue* source, Queue* dest) {
    while (!queue_is_empty(source)) {
        Process* p = queue_pop(source);
        process_set_current_queue(p, 0); // Vuelven a la prioridad más alta
        queue_push(dest, p);
    }
}

// Funciones auxiliares para capturar el estado de las colas y procesos en el trace
static void trace_copy_pid(char* destination, size_t destination_size, Process* process) {
    if (process) snprintf(destination, destination_size, "%s", process_get_pid(process));
    else snprintf(destination, destination_size, "-");
}

static void trace_capture_io(char* destination, size_t destination_size, Queue* io_queue) {
    int count = queue_get_size(io_queue);
    destination[0] = '\0';

    for (int i = 0; i < count; i++) {
        Process* process = queue_pop(io_queue);
        if (i > 0) strncat(destination, ",", destination_size - strlen(destination) - 1);
        strncat(destination, process_get_pid(process), destination_size - strlen(destination) - 1);
        queue_push(io_queue, process);
    }

    if (destination[0] == '\0') snprintf(destination, destination_size, "-");
}

static void scheduler_capture_trace(MLFQScheduler* scheduler) {
    if (scheduler->trace_size == scheduler->trace_capacity) {
        int new_capacity = scheduler->trace_capacity == 0 ? 16 : scheduler->trace_capacity * 2;
        SchedulerTraceEntry* new_trace = (SchedulerTraceEntry*)realloc(
            scheduler->trace, (size_t)new_capacity * sizeof(SchedulerTraceEntry));
        if (!new_trace) return;
        scheduler->trace = new_trace;
        scheduler->trace_capacity = new_capacity;
    }

    SchedulerTraceEntry* entry = &scheduler->trace[scheduler->trace_size++];
    entry->time = scheduler->current_time;
    trace_copy_pid(entry->q0, sizeof(entry->q0),
        scheduler->current_running && scheduler->current_queue_level == 0
            ? scheduler->current_running : NULL);
    trace_copy_pid(entry->q1, sizeof(entry->q1),
        scheduler->current_running && scheduler->current_queue_level == 1
            ? scheduler->current_running : NULL);
    trace_copy_pid(entry->q2, sizeof(entry->q2),
        scheduler->current_running && scheduler->current_queue_level == 2
            ? scheduler->current_running : NULL);
    trace_capture_io(entry->qio, sizeof(entry->qio),
        io_manager_get_waiting_processes(scheduler->io_mgr));
}

// Funciones del Scheduler
void scheduler_add_process(MLFQScheduler* s, Process* p) {
    process_set_state(p, READY);
    process_set_current_queue(p, 0); // Todos los procesos inician en la cola de mayor prioridad
    queue_push(s->q0, p);
}

void scheduler_tick(MLFQScheduler* s) {
    // 1. Priority Boost: cada S ciclos, todos los procesos vuelven a la cola de mayor prioridad
    if (s->boost_interval > 0 && s->current_time > 0 && (s->current_time % s->boost_interval == 0)) {
        move_queue(s->q1, s->q0);
        move_queue(s->q2, s->q0);
        
        // Si hay un proceso corriendo, también se le reinicia su nivel de prioridad para el próximo turno
        if (s->current_running) {
            process_set_current_queue(s->current_running, 0);
            s->current_queue_level = 0;
        }
    }

    scheduler_capture_trace(s);

    // 2. Gestionar dispositivos de I/O
    io_manager_tick(s->io_mgr);
    
    // Recuperar procesos que ya terminaron su I/O en este ciclo
    Queue* returned_from_io = io_manager_get_ready_processes(s->io_mgr);
    while (!queue_is_empty(returned_from_io)) {
        Process* p = queue_pop(returned_from_io);
        int q_level = process_get_current_queue(p);
        
        // Vuelven a la cola en la que estaban antes de bloquearse
        if (q_level == 0) queue_push(s->q0, p);
        else if (q_level == 1) queue_push(s->q1, p);
        else queue_push(s->q2, p);
    }

    // 3. Gestionar el proceso actual
    if (s->current_running) {
        // Ejecuta un ciclo de reloj discreto
        process_execute_cycle(s->current_running, s->current_time);
        s->current_quantum_used++;

        if (process_is_finished(s->current_running)) {
            // El proceso terminó su ejecución total
            process_set_state(s->current_running, TERMINATED);
            queue_push(s->terminated, s->current_running);
            s->current_running = NULL;
        } 
        else if (process_needs_io(s->current_running)) {
            // Cede la CPU de forma voluntaria para I/O antes de agotar su quantum y no baja de nivel
            process_start_io(s->current_running);
            io_manager_add_process(s->io_mgr, s->current_running);
            s->current_running = NULL; // Libera la CPU
        }
        else if (s->current_quantum_used >= QUANTUMS[s->current_queue_level]) {
            // Si el proceso consume todo su quantum, baja un nivel
            int next_q = (s->current_queue_level < 2) ? (s->current_queue_level + 1) : 2;
            process_set_current_queue(s->current_running, next_q);
            process_set_state(s->current_running, READY);
            
            if (next_q == 0) queue_push(s->q0, s->current_running);
            else if (next_q == 1) queue_push(s->q1, s->current_running);
            else queue_push(s->q2, s->current_running);
            
            s->current_running = NULL; // Libera la CPU para que el scheduler elija de nuevo
        }
    }

    // 4. Ejecutar siempre el proceso en la cola de mayor prioridad disponible usando Round Robin
    if (!s->current_running) {
        if (!queue_is_empty(s->q0)) {
            s->current_running = queue_pop(s->q0);
            s->current_queue_level = 0;
        } else if (!queue_is_empty(s->q1)) {
            s->current_running = queue_pop(s->q1);
            s->current_queue_level = 1;
        } else if (!queue_is_empty(s->q2)) {
            s->current_running = queue_pop(s->q2);
            s->current_queue_level = 2;
        }
        
        if (s->current_running) {
            process_set_state(s->current_running, RUNNING);
            s->current_quantum_used = 0; // Reiniciar contador de quantum para el nuevo turno
        }
    }

    // 5. Avanzar el tiempo de la simulación por ciclos de reloj
    s->current_time++;
}

int scheduler_is_done(MLFQScheduler* s) {
    // La simulación termina cuando todas las colas están vacías y no hay procesos corriendo
    return !s->current_running && 
           queue_is_empty(s->q0) && 
           queue_is_empty(s->q1) && 
           queue_is_empty(s->q2) &&
           io_manager_is_empty(s->io_mgr);
}

Queue* scheduler_get_terminated(MLFQScheduler* s) {
    return s->terminated;
}

int scheduler_get_current_time(MLFQScheduler* s) {
    return s->current_time;
}

int scheduler_get_trace_size(MLFQScheduler* s) {
    return s->trace_size;
}

const SchedulerTraceEntry* scheduler_get_trace_entry(MLFQScheduler* s, int index) {
    if (index < 0 || index >= s->trace_size) return NULL;
    return &s->trace[index];
}