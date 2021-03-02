/*
 * The infamous task
 *
 * (c) christoph irrenfried, chi86
 *
 */

#ifndef TASK_H
#define TASK_H

#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

/*
 * Task
 */
struct Task {
  int id;

  char* log;
  char* name;   // name given by user
  char* qname;  // name in queue (task{ID})
  char* cmd;

  char* env[7];
  char* logname;

  uid_t uid;        /* user ID */
  gid_t gid;        /* group ID */

  // char* owner;
  clock_t time_queued;
  clock_t time_running;
  clock_t time_finished;
  
  int nproc;

  int state;

  char* cwd;
  int pid;

  char* argv[3];
};
typedef struct Task Task;

/*
 * task members
 */
int task_id(Task *task);

char* task_time(Task *task);
void task_init(Task *task);
void task_addCMD(Task *task,char* CMD);
void task_addCWD(Task *task,char* CWD);
void task_setup(Task *task);

void task_write(Task *task, char* root);

Task * task_read(char* root);


void task_bin(Task *task);

#endif // TASK_H
