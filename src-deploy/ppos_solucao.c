// Arquivo criado para resolução do projeto

#include "ppos.h"
#include "ppos-data.h"
#include "ppos-core-globals.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos-disk-manager.h"
#include <unistd.h>

#define MAX_TASKS 100

unsigned int _systemTime = 0;

typedef struct
{
    int static_prio;
    int dynamic_prio;
    int quantum;
    int is_system_task;

    unsigned int execution_time_start;
    unsigned int processor_time;
    unsigned int activations;

} ppos_task_info_t;

ppos_task_info_t task_infos[MAX_TASKS];

// define prioridade de uma tarefa
void task_setprio(task_t *task, int prio)
{
    if(!task)
        task = taskExec;

    if(prio>20)
        prio = 20;
    if(prio<-20)
        prio = -20;

    task_infos[task->id].static_prio = prio;
    task_infos[task->id].dynamic_prio = prio;
}

// pega prioridade de tarefa
int task_getprio(task_t *task)
{
    if(!task)
        return task_infos[taskExec->id].static_prio;
    return task_infos[task->id].static_prio;
}

task_t* scheduler()
{
    if (queue_size((queue_t*)readyQueue) == 0)
        return NULL;

    int first_prio = task_infos[readyQueue->id].dynamic_prio;
    task_t* aux = readyQueue->next;
    int all_same_prio = 1;
    for (int i = 1; i < queue_size((queue_t*)readyQueue); i++) 
    {
        if (task_infos[aux->id].dynamic_prio != first_prio)
        {
            all_same_prio = 0;
            break;
        }
        aux = aux->next;
    }

    if (all_same_prio)
        return readyQueue;

    task_t* chosen_task = readyQueue;
    task_t* current_task = readyQueue->next;
    for (int i = 1; i < queue_size((queue_t*)readyQueue); i++) 
    {
        if (task_infos[current_task->id].dynamic_prio < task_infos[chosen_task->id].dynamic_prio) 
        {
            chosen_task = current_task;
        }
        current_task = current_task->next;
    }

    current_task = readyQueue;
    for (int i = 0; i < queue_size((queue_t*)readyQueue); i++) 
    {
        if (current_task != chosen_task) 
        {
            task_infos[current_task->id].dynamic_prio--;
            if (task_infos[current_task->id].dynamic_prio < -20)
                task_infos[current_task->id].dynamic_prio = -20;
        }
        current_task = current_task->next;
    }

    task_infos[chosen_task->id].dynamic_prio = task_infos[chosen_task->id].static_prio;
    return chosen_task;

}

void tick_handler(int signum) 
{
    _systemTime++;

    if (taskExec && !task_infos[taskExec->id].is_system_task) 
    {
        task_infos[taskExec->id].processor_time++;
        task_infos[taskExec->id].quantum--;

        if (task_infos[taskExec->id].quantum <= 0) 
        {
            queue_append((queue_t**)&readyQueue, (queue_t*)taskExec);
            task_switch(taskDisp);
            //task_yield();
            // num funciona nem com o yield e nem colocando no pelo kkkkkkkkk
        }
    }
}

unsigned int systime()
{
    return _systemTime;
}


diskrequest_t* disk_scheduler() {
  return NULL;
}