#ifndef __PPOS_DISK_H__
#define __PPOS_DISK_H__

#include "ppos-data.h"
#define FCFS_POLICY   0
#define SSTF_POLICY   1
#define CSCAN_POLICY  2

extern int disk_scheduler_policy;

int disk_mgr_init(int *numBlocks, int *blockSize);

int disk_block_read(int block, void* buffer);

int disk_block_write(int block, void* buffer);

diskrequest_t* disk_scheduler();

diskrequest_t* fcfs_scheduler();

diskrequest_t* sstf_scheduler();

diskrequest_t* cscan_scheduler();

long get_total_head_movement();

#endif