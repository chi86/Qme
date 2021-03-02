/*
 * All about the queue
 *
 * (c) christoph irrenfried, chi86
 *
 */

#include "Queue.h"

/*
 * initialize queue
 */
void queue_init(Queue* queue, char* path)
{
  char fname[1000];
  FILE *fp;

  queue->NPROC=get_nprocs_conf();
  queue->PROC=0;
  
  queue->queued=NULL;
  queue->running=NULL;
  
  queue->path=path;

  // read version file
  strcpy(fname,queue->path);
  strcat(fname,"/info/version");
  if( access( fname, F_OK ) == 0 ) {
    // file exist
    fp=fopen(fname,"r");
    if(fscanf(fp, "%f", &queue->version));
    fclose(fp);
  } else {
    printf("can't read queue version file\n");
  }

  // read taskid, last task beeing processed
  strcpy(fname,queue->path);
  strcat(fname,"/info/taskid");
  if( access( fname, F_OK ) == 0 ) {
    // file exist
    fp=fopen(fname,"r");
    if(fscanf(fp, "%d", &queue->taskid));
    fclose(fp);
  } else {
    printf("can't read taskid file\n");
  }
}

/*
 * update last task id
 */
void queue_update(Queue* queue)
{
  char fname[1000];
  FILE *fp;

  // create/read taskid
  strcpy(fname,queue->path);
  strcat(fname,"/info/taskid");
  fp=fopen(fname,"w");
  fprintf(fp, "%d",queue->taskid);
  fclose(fp);

}

/*
 * clean out list of tasks from QUEUED & RUNNING
 */
void queue_stop(Queue* queue)
{
  list_t * current;
  list_t * previous;
  
  previous= queue->queued;
  while (previous != NULL) {
    current=previous->next;
    task_bin(previous->task);
    free(previous->task);
    free(previous);
    previous=current;
  }
  previous = queue->running;;
  while (previous != NULL) {
    current=previous->next;
    task_bin(previous->task);
    free(previous->task);
    free(previous);
    previous=current;
  }
}

/*
 * return queue version
 */ 
float queue_version(Queue* queue)
{
  return queue->version;
}

/*
 * return id of the last processed task
 */ 
int queue_taskid(Queue* queue) {
  return queue->taskid;
}
/*
 * update id of the last processed task
 */ 
void queue_udateTaskId(Queue* queue, int id)
{
  char fname[1000];
  FILE *fp;

  // create/read taskid
  strcpy(fname,queue->path);
  strcat(fname,"/info/taskid");
  fp=fopen(fname,"w");
  fprintf(fp,"%d",id);
  fclose(fp);
}

/*
 * set the number of currently used cpus
 */ 
void queue_setNPROC(Queue* queue)
{
  queue->NPROC=get_nprocs_conf();
}
