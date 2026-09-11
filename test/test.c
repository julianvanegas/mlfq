#include <stdio.h>
#include <assert.h>
#include "../src/domain/process.h"
#include "../src/application/scheduler.h"

// Prueba de cálculo de métricas obligatorias
void test_process_metrics() {
    printf("Ejecutando test_process_metrics\n");
    // arrival=2, burst=3
    Process* p = process_create("P1", 2, 3, 0, 0);
    
    // Simulamos que el scheduler lo atiende desde el ciclo 4 al 6
    process_execute_cycle(p, 4); // Response time se fija aquí (4 - arrival(2) = 2)
    process_execute_cycle(p, 5);
    process_execute_cycle(p, 6); // Termina aquí. Finish time = 7.
    
    assert(process_is_finished(p) == 1);
    
    // Verificación de fórmulas
    // Response = first_response (4) - arrival (2) = 2
    assert(process_get_response_time(p) == 2);
    
    // Turnaround = finish (7) - arrival (2) = 5
    assert(process_get_turnaround_time(p) == 5);
    
    // Waiting = turnaround (5) - burst (3) = 2
    assert(process_get_waiting_time(p) == 2);
    
    process_destroy(p);
    printf("-> test_process_metrics PASO OK.\n");
}

// Prueba de la lógica de democión (quantum agotado)
void test_demotion_logic() {
    printf("Ejecutando test_demotion_logic\n");
    // Priority Boost lejano para que no interfiera
    MLFQScheduler* sched = scheduler_create(100); 
    
    // arrival=0, burst=5
    Process* p = process_create("P1", 0, 5, 0, 0);
    scheduler_add_process(sched, p);
    
    // Verificamos que inicia en Q0
    assert(process_get_current_queue(p) == 0);
    
    // El quantum de Q0 es 1 ciclo
    scheduler_tick(sched); // Tick 1: Se asigna a current_running
    scheduler_tick(sched); // Tick 2: Se ejecuta y aplica democión a Q1 (quantum 1 agotado)
    
    // Verificamos la democión a Q1
    assert(process_get_current_queue(p) == 1);
    
    scheduler_destroy(sched);
    printf("-> test_demotion_logic PASO OK.\n");
}

// Prueba de Priority Boost
void test_priority_boost() {
    printf("Ejecutando test_priority_boost...\n");
    // Boost cada 3 ciclos
    MLFQScheduler* sched = scheduler_create(3); 
    Process* p = process_create("P1", 0, 10, 0, 0);
    scheduler_add_process(sched, p);
    
    // Tick 1: Se asigna desde Q0
    // Tick 2: Se ejecuta en Q0, se aplica democión a Q1, se asigna desde Q1
    scheduler_tick(sched); 
    scheduler_tick(sched);
    assert(process_get_current_queue(p) == 1);
    assert(process_get_remaining(p) == 9);
    
    // Tick 3: Se ejecuta en Q1
    scheduler_tick(sched);
    assert(process_get_current_queue(p) == 1);
    assert(process_get_remaining(p) == 8);
    
    // Tick 4: Priority Boost ocurre (current_time = 3, 3 % 3 == 0)
    // Se mueve a Q0, se ejecuta, se aplica democión a Q1, se asigna desde Q1
    scheduler_tick(sched);
    assert(process_get_current_queue(p) == 1);
    assert(process_get_remaining(p) == 7);
    
    scheduler_destroy(sched);
    printf("-> test_priority_boost PASO OK.\n");
}

int main() {
    test_process_metrics();
    test_demotion_logic();
    test_priority_boost();
    printf("--- LAS PRUEBAS PASARON ---\n");
    return 0;
}