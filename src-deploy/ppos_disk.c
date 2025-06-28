#include "ppos.h"
#include "ppos-disk-manager.h"
#include "disk-driver.h"
#include "ppos-core-globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAX_TASKS 100
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

extern ppos_task_info_t task_infos[];
static disk_t disk;
static task_t disk_manager_task;
static diskrequest_t *request_queue;
static semaphore_t disk_sema; // optei pelo semaforo para simplificar a sincronização e controlar o acesso do disco
static diskrequest_t *current_request = NULL;
static semaphore_t sem_disk;  

static void disk_manager_body(void *arg);
static void disk_signal_handler(int signum);

// função para inicializar o disco, criar a tarefa gerenciadora e preparar o tratador de sinais
int disk_mgr_init(int *numBlocks, int *blockSize)
{
    if(disk_cmd(DISK_CMD_INIT, 0, 0) < 0) // disk init
        return -1;

    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0); // params de disco
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    if(*numBlocks < 0 || *blockSize < 0)
        return -1;

    request_queue = NULL; // fila de pedidos

    task_create(&disk_manager_task, disk_manager_body, NULL); // tarefa para gerenciar o disco
    task_infos[disk_manager_task.id].is_system_task = 1; // tem q ser tarefa de sistema, obviamente

    struct sigaction action; // tratador de sinal semelhante ao feito no ppos-core-aux
    action.sa_handler = disk_signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGUSR1, &action, 0) < 0) // registrando o handler pro sinal do disco
        return -1;

    if(sem_create(&disk_sema, 1) != 0) // enfim criado o semaforo pra sincronia de acesso (obs: iniciado em 1, indicando q o disco ta livre)
        return -1;

    return 0;
}

int disk_block_read(int block, void* buffer) // funçãp de leitura
{
    sem_down(&disk_sema); // request pro disco, da acesso pra fila de pedidos.

    diskrequest_t *request = malloc(sizeof(diskrequest_t)); //tudo aqui embaixo é a estrutura de como funciona o pedido
    if(!request)
        return -1;

    request->task = taskExec;
    request->operation = DISK_CMD_READ; // leitura yaaay!
    request->block = block;
    request->buffer = buffer;

    queue_append((queue_t**)&request_queue, (queue_t*)request); // coloca no fim da fila de acorod com a politica estabelecida

    sem_up(&disk_sema); // libera o semaforo

    task_resume(&disk_manager_task); // acorda a tarefa manager q ta dormindo

    task_yield();

    return 0;

}

int disk_block_write(int block, void* buffer) // função de escrevura, mesma logica da de leitura, só muda a operação
{
    sem_down(&disk_sema);

    diskrequest_t *request = malloc(sizeof(diskrequest_t));
    if(!request)
        return -1;
    request->task = taskExec;
    request->operation = DISK_CMD_WRITE; // escrita yaaayy
    request->block = block;
    request->buffer = buffer;

    queue_append((queue_t**)&request_queue, (queue_t*)request);

    sem_up(&disk_sema);

    task_resume(&disk_manager_task);
    
    task_yield();

    return 0;
}

static void disk_manager_body(void *arg)
{
    while(1)
    {
        sem_down(&sem_disk); // pede acesso ao disco

        if(request_queue) // ve se tem gente na fila de pedido
        {
            current_request = request_queue;
            request_queue = (diskrequest_t*)request_queue->next;
            current_request->next = NULL;
            current_request->prev = NULL;

            if(current_request->operation == DISK_CMD_READ)
                memset(current_request->buffer, 0, disk.blockSize);
            disk_cmd(current_request->operation, current_request->block, current_request->buffer); // envia o comando
        }
        else // se num tem pedido e fica suave até ser acordado
        {
            sem_up(&sem_disk);
            task_suspend(&disk_manager_task, NULL);
            task_yield();
        }
    }
}

static void disk_signal_handler(int signum)
{
    if(current_request)
    {
        printf("DEBUG: Disco terminou. Buffer contém: [%s]\n", (char*)current_request->buffer);
        task_resume(current_request->task); // acorda tarefga de usuario e libera memoria do pedido
        free(current_request);
        current_request = NULL;
    }

    sem_up(&sem_disk); // libera o semaforo
}