/*
 * All about the queue
 *
 * (c) christoph irrenfried, chi86
 *
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "unistd.h"

#include <sys/sysinfo.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>


#include "Task.h"
#include "List.h"

/*
 * Queue
 */
struct Queue {
  char* path;
  float version;
  
  // max available CPUs
  int NPROC;
  // currently used CPUs
  int PROC;
  
  // id of last task
  int taskid;

  list_t * queued;
  list_t * running;
};
typedef struct Queue Queue;

/*
 * Queue members
 */
void queue_init(Queue* queue, char* path);
void queue_update(Queue* queue);
void queue_stop(Queue* queue);

float queue_version(Queue* queue);
int queue_taskid(Queue* queue);

void queue_udateTaskId(Queue* queue, int id);

void queue_setNPROC(Queue* queue);

#endif // QUEUE_H
