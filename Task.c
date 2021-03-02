/*
 * The infamous task
 *
 * (c) christoph irrenfried, chi86
 *
 */

#include "Task.h"

/*
 * Update task id
 */
int task_id(Task *task)
{
  return task->id;
}

/*
 * Initialize task
 */
void task_init(Task *task)
{  
  task->pid=0;

  task->state=0;
  
  task->time_queued=time(NULL);
  task->time_running=time(NULL);
  task->time_finished=time(NULL);
  
  task->name=malloc(200*sizeof(char));
  task->qname=malloc(200*sizeof(char));
  task->cmd=malloc(200*sizeof(char));
  task->cwd=malloc(200*sizeof(char));
  task->log=malloc(200*sizeof(char));
  
  task->env[0]=malloc(200*sizeof(char));
  task->env[1]=malloc(200*sizeof(char));
  task->env[2]=malloc(200*sizeof(char));
  task->env[3]=malloc(200*sizeof(char));
  task->env[4]=malloc(200*sizeof(char));
  task->env[5]=malloc(200*sizeof(char));
  task->env[6]=0;
  
  task->logname=malloc(200*sizeof(char));

  task->argv[0]=malloc(20*sizeof(char));
  task->argv[1]=malloc(200*sizeof(char));
  strcpy(task->argv[0],"/bin/bash");
  task->argv[2]=NULL;
}


/*
 * setup argva and logging
 */
void task_setup(Task *task)
{
  char log[1000];
  
  strcpy(task->argv[1],task->cwd);
  strcat(task->argv[1],task->cmd);
  
  sprintf(log, "%stask%d.log", task->cwd,task->id);
  
  strcpy(task->log,log);
}

/*
 * add CMD to task
 */
void task_addCMD(Task *task,char* CMD)
{
  strcpy(task->cmd,CMD);
  strcpy(task->argv[1],CMD);
}

/*
 * add CWD to task
 */
void task_addCWD(Task *task,char* CWD)
{
  strcpy(task->cwd,CWD);
  strcpy(task->argv[1],CWD);
}

/*
 * discard task
 */
void task_bin(Task *task)
{
  free(task->argv[0]);
  free(task->argv[1]);
  
  free(task->env[0]);
  free(task->env[1]);
  free(task->env[2]);
  free(task->env[3]);
  free(task->env[4]);
  free(task->env[5]);
  
  free(task->logname);
  
  free(task->name);
  free(task->qname);
  free(task->cmd);
  free(task->cwd);
  free(task->log);
}

/*
 * write task to file
 */
void task_write(Task *task, char* root)
{
  char fname[1000];
  FILE *fp;
  
  sprintf(fname, "%s", root);

  fp=fopen(fname,"w");
  
  fprintf(fp, "%20s: %d\n","id",task->id);
  fprintf(fp, "%20s: %s\n","name",task->name);
  fprintf(fp, "%20s: %d\n","pid",task->pid);
  fprintf(fp, "%20s: %s\n","cmd",task->cmd);
  fprintf(fp, "%20s: %s\n","cwd",task->cwd);
  fprintf(fp, "%20s: %d\n","nproc",task->nproc);
  
  fprintf(fp, "%20s: %s\n","env",task->env[0]);
  fprintf(fp, "%20s: %s\n","env",task->env[1]);
  fprintf(fp, "%20s: %s\n","env",task->env[2]);
  fprintf(fp, "%20s: %s\n","env",task->env[3]);
  fprintf(fp, "%20s: %s\n","env",task->env[4]);
  fprintf(fp, "%20s: %s\n","env",task->env[5]);
  
  fprintf(fp, "%20s: %s\n","logname",task->logname);
  fprintf(fp, "%20s: %d\n","uid",task->uid);
  fprintf(fp, "%20s: %d\n","gid",task->gid);
  
  fprintf(fp, "%20s: %ld\n","t_queued",task->time_queued);
  fprintf(fp, "%20s: %ld\n","t_running",task->time_running);
  fprintf(fp, "%20s: %ld\n","t_finshed",task->time_finished);
  
  fclose(fp);
}

/*
 * read task from file
 */
Task * task_read(char* root)
{
  char fname[1000];
  FILE *fp;

  char tmp[2000];

  Task *task=malloc(sizeof(Task));
  task_init(task);
  
  sprintf(fname, "%s", root);

  fp=fopen(fname,"r");

  if(fscanf(fp, "%s %d\n",tmp,&task->id));
  if(fscanf(fp, "%s %s\n",tmp,task->name));
  if(fscanf(fp, "%s %d\n",tmp,&task->pid));
  if(fscanf(fp, "%s %s\n",tmp,task->cmd));
  if(fscanf(fp, "%s %s\n",tmp,task->cwd));
  if(fscanf(fp, "%s %d\n",tmp,&task->nproc));
  
  if(fscanf(fp, "%s %s\n",tmp,task->env[0]));
  if(fscanf(fp, "%s %s\n",tmp,task->env[1]));
  if(fscanf(fp, "%s %s\n",tmp,task->env[2]));
  if(fscanf(fp, "%s %s\n",tmp,task->env[3]));
  if(fscanf(fp, "%s %s\n",tmp,task->env[4]));
  if(fscanf(fp, "%s %s\n",tmp,task->env[5]));
  
  if(fscanf(fp, "%s %s\n",tmp,task->logname));
  if(fscanf(fp, "%s %d\n",tmp,&task->uid));
  if(fscanf(fp, "%s %d\n",tmp,&task->gid));

  if(fscanf(fp, "%s %ld\n",tmp,&task->time_queued));
  if(fscanf(fp, "%s %ld\n",tmp,&task->time_running));
  
  if(fscanf(fp, "%s %ld\n",tmp,&task->time_finished));
  fclose(fp);

  sprintf(task->qname, "task%d", task->id);
  
  task_setup(task);
  
  return task;
}
