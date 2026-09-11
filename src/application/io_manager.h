#ifndef IO_MANAGER_H
#define IO_MANAGER_H
#include "../domain/process.h"
#include "../domain/queue.h"

typedef struct IOManager IOManager;

IOManager* io_manager_create();
void io_manager_destroy(IOManager* mgr);
void io_manager_add_process(IOManager* mgr, Process* p);
void io_manager_tick(IOManager* mgr);
Queue* io_manager_get_ready_processes(IOManager* mgr);
Queue* io_manager_get_waiting_processes(IOManager* mgr);
int io_manager_is_empty(IOManager* mgr);

#endif