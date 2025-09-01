/*
 * A linked list for tasks
 *
 * (c) christoph irrenfried, chi86
 *
 */

#ifndef LIST_H
#define LIST_H

#include "stdio.h"
#include <stdlib.h>
#include <dirent.h>

#include "Task.h"
#include "Progress.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
 * Linked list
 */
typedef struct list {
  Task *task;
  struct list * next;
} list_t;

/*
 * linked list members
 */
void print_list(list_t ** head,int verbose);
void push_list(list_t ** head, Task *task);
Task * pop_list(list_t ** head);
void removeID_list(list_t ** head, int id);
Task * getID_list(list_t ** head, int id);
void moveID_list(list_t ** head,list_t ** sink, int id);
bool existsID_list(const list_t *head, int id);

#endif // TASK_H
