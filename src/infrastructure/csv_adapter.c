#include "csv_adapter.h"
#include <stdio.h>

void export_results_csv(const char* filename, Queue* terminated_processes) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir %s para escribir.\n", filename);
        return;
    }

    // Formato de salida requerido.
    fprintf(file, "PID ; Arrival ; Burst ; Start ; Finish ; Response ; Turnaround ; Waiting\n");

    // No modificamos la cola original, solo leemos sus elementos
    Queue* temp_q = queue_create();
    
    while (!queue_is_empty(terminated_processes)) {
        Process* p = queue_pop(terminated_processes);
        
        fprintf(file, "%s ; %d ; %d ; %d ; %d ; %d ; %d ; %d\n",
            process_get_pid(p),
            process_get_arrival(p),
            process_get_burst(p),
            process_get_response_time(p) + process_get_arrival(p), // Start time
            process_get_turnaround_time(p) + process_get_arrival(p), // Finish time
            process_get_response_time(p),
            process_get_turnaround_time(p),
            process_get_waiting_time(p)
        );
        
        queue_push(temp_q, p);
    }

    // Restaurar la cola original para no destruir los datos
    while (!queue_is_empty(temp_q)) {
        queue_push(terminated_processes, queue_pop(temp_q));
    }
    
    queue_destroy(temp_q);
    fclose(file);
}