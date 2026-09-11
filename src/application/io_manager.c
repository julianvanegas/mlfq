#include "io_manager.h"
#include <stdlib.h>

struct IOManager {
    Queue* waiting_processes;
    Queue* ready_to_return;
};

IOManager* io_manager_create() {
    IOManager* mgr = (IOManager*)malloc(sizeof(IOManager));
    mgr->waiting_processes = queue_create();
    mgr->ready_to_return = queue_create();
    return mgr;
}

void io_manager_destroy(IOManager* mgr) {
    queue_destroy(mgr->waiting_processes);
    queue_destroy(mgr->ready_to_return);
    free(mgr);
}

void io_manager_add_process(IOManager* mgr, Process* p) {
    queue_push(mgr->waiting_processes, p);
}

void io_manager_tick(IOManager* mgr) {
    int size = queue_get_size(mgr->waiting_processes);
    
    // Iteramos sobre todos los dispositivos en paralelo
    for (int i = 0; i < size; i++) {
        Process* p = queue_pop(mgr->waiting_processes);
        process_execute_io_cycle(p);
        
        if (process_is_io_finished(p)) {
            process_set_state(p, READY);
            queue_push(mgr->ready_to_return, p);
        } else {
            queue_push(mgr->waiting_processes, p); // Aún no termina, vuelve a esperar
        }
    }
}

Queue* io_manager_get_ready_processes(IOManager* mgr) {
    return mgr->ready_to_return;
}

Queue* io_manager_get_waiting_processes(IOManager* mgr) {
    return mgr->waiting_processes;
}

int io_manager_is_empty(IOManager* mgr) {
    return queue_is_empty(mgr->waiting_processes) && queue_is_empty(mgr->ready_to_return);
}