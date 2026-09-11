#include <stdio.h>
#include "application/scheduler.h"
#include "domain/process.h"
#include "infrastructure/csv_adapter.h"
#include "infrastructure/console_adapter.h"

int main() {
    MLFQScheduler* scheduler = scheduler_create(10);

    Process* incoming[] = {
        // PID, arrival, burst, io_freq, io_duration
        process_create("P1", 0, 1, 0, 0),
        process_create("P2", 11, 5, 2, 5),
        process_create("P3", 18, 8, 3, 10),
        process_create("P4", 3, 12, 4, 10)
    };
    int num_incoming = 4;
    int incoming_added = 0;

    printf("Iniciando simulacion MLFQ\n");

    while (!scheduler_is_done(scheduler) || incoming_added < num_incoming) {
        int current_time = scheduler_get_current_time(scheduler);

        for (int i = 0; i < num_incoming; i++) {
            if (incoming[i] != NULL && process_get_arrival(incoming[i]) == current_time) {
                scheduler_add_process(scheduler, incoming[i]);
                incoming[i] = NULL; 
                incoming_added++;
            }
        }
        scheduler_tick(scheduler);
    }

    // Exportar archivo .csv con los resultados
    Queue* terminated = scheduler_get_terminated(scheduler);
    export_results_csv("results.csv", terminated);
    export_execution_trace_console(scheduler, stdout);
    
    printf("Simulacion completada. Resultados en results.csv\n");

    while (!queue_is_empty(terminated)) {
        process_destroy(queue_pop(terminated));
    }
    scheduler_destroy(scheduler);

    return 0;
}