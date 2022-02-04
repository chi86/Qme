/*
 * Queue
 * A simple and fast task queuing system
 * 
 * The basic approach is that a task can hold four states, which are
 * INIT -> QUEUED -> RUNNING -> FINISHED
 *
 * by submitting a task, it will enter the INIT state.
 * The Queue deamon (openrc) immediately picks it up and changes the state to QUEUED.
 * If the needed resources are available and the said task is next in line
 * the task will enter the RUNNING state, with the associated execution of the task.
 * When the execution is completed the task reaches the final FINISHED state.
 *
 +
 + v1.2: an additional nproc file can be specified in /var/spool/Qme/info/
 +       specifying the max number of used procs
 +
 * 
 * (c) christoph irrenfried, chi86
 *
 */

#include "stdio.h"

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>

#include "Task.h"
#include "List.h"
#include "Queue.h"

/*
 * global assignation of paths
 */
#define INSTALL_DIR "/var/spool/Qme"
#define LOCK_FILE "/var/spool/Qme/info/queue.lock"
#define LOG_FILE "/var/spool/Qme/info/Qme.log"

/*
 * refresh interval of the queuing system
 */
#define WAITTIME 1

/*
 * function declaration of queue related methodes
 */
void create_tasks();
void retrive_tasks(Queue *queue,char* root);
void run_queue();
void addTo_queue(char * file, char * name, char * script, char * nproc);
void list_queue();
void queue_cli();
void deleteFrom_queue(char * pidC);
void list_tasks(char * input,char * state);
void add_tasks_to_queue(list_t ** qlist, char * input,int state);

/*
 * global helpers
 */
void _help();
void _deamonize();
void _logging(char* buffer);


/*
 * main routine
 */
int main(int argc, char* argv[])
{
  int i;
  if(argc == 1) {
    // If no command line arguments are passed, execute the cli
    queue_cli();
  }
  else {
    for(i=1;i<argc;i++) {
      /* printf("\t *** argument : %s\n",argv[i]); */

      if(strcmp(argv[i],"-h")==0) {
	// display help
	_help();
	return 0;
      }
      else if(strcmp(argv[i],"-a")==0) {
	// add a task to the queue
	if(argc==5) {
	  addTo_queue(argv[1],argv[i+1],argv[i+2],argv[i+3]);
	}
	return 0;
      }
      else if(strcmp(argv[i],"-d")==0) {
	// delete a task from the queue
	if(argc==3) {
	  deleteFrom_queue(argv[i+1]);
	}
	return 0;
      }
      else if(strcmp(argv[i],"-Y")==0) {
	// directly run queue
        run_queue();
	return 0;
      }
      else if(strcmp(argv[i],"-l")==0) {
	// list all tasks
        list_queue();
	return 0;
      }
      else if(strcmp(argv[i],"-X")==0) {
	// demonize
	_deamonize();
	return 0;
      }
      else {
	printf("unkown flag\n");
	_help();
	return 5; // error code "unkown flag"
      }
    }
  }

  // show is over, go home!
  return 0;
}


/*
 * help message
 */
void _help()
{
  printf("Queuing system: Qme\n");
  printf("---------------------\n\n");
  
  printf("Usage: Qme [FLAG] [ARGUMENTS]\n\n");

  printf("FLAG     interpretation\n");
  printf("  -h     Help (this!)\n");
  printf("  -Y     run Queue\n");
  printf("  -X     Demonize the Queue\n");
  printf("  -a     submit add new task to queue\n");
  printf("         Arguments:\n");
  printf("         NAME SCRIPT.sh NPROC\n");
  printf("         Default:\n");
  printf("         RANSrun Allrun.sh 1\n");
  printf("  -d     delete task from queue\n");
  printf("         Arguments:\n");
  printf("         ID\n");
  printf("  -l     list all tasks\n");

  printf("\n\n");
}


/*
 * log informations in the predefined LOG_FILE
 */
void _logging(char* buffer) {
  
  FILE *logfile = fopen(LOG_FILE, "a");

  time_t ltime=time(NULL);
  fprintf(logfile, "[%.24s] %s", asctime(localtime(&ltime)),buffer);
  fclose(logfile);
}

/*
 * can be used as deamon independently from openrc or other init 
 */
void _deamonize()
{
  pid_t pid, sid;
  int fd,lfp;
  char str[10];
  
  // Fork off
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  // Did it work?
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }  
                
  // Open logging
  fd = open(LOG_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  dup2(fd, 1);
  dup2(fd, 2);
  close(fd);

  // set child sid
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  // lockfile, with permissions
  lfp = open(LOCK_FILE,O_RDWR|O_CREAT,0640);
  if(lfp < 0)
    exit(1);
  if(lockf(lfp,F_TLOCK,0) < 0)
    exit(1);
  
  sprintf(str,"%d\n",getpid());
  if(write(lfp,str,strlen(str)));
  close(lfp);
  
  // start queue
  run_queue();
}


/*
 * delete a task by pid
 */
void deleteFrom_queue(char * pidC)
{
  pid_t pid=atoi(pidC);
  int temppgid=getpgid(pid);
  printf("task to delete %d %d\n",pid,temppgid);
  killpg(temppgid,SIGKILL);
  // killpg(temppgid,SIGTERM);
}


/*
 * add ned task to queue, start at init_dir
 * task enters the INIT state
 */
void addTo_queue(char * file, char * name, char * script, char * nproc)
{
  char* root=INSTALL_DIR;
  char fname[1000];
  char cwd[PATH_MAX];
  
  Queue *queue = malloc(sizeof(Queue));
  queue_init(queue,root);


  printf("Adding task to queue\n");

  Task * task=malloc(sizeof(Task));
  task_init(task);

  char login[256];

  // get login of task submitter
  getlogin_r(login, 256);

  // give me the stats
  struct passwd *pw = getpwnam(login);
  if(pw==NULL) {
    printf("User %s does not exists!\n",login);
  }

  // fill the task with all needed infomations
  task->id=queue->taskid;
  queue->taskid++;
  printf("Task id             : %d\n", task->id);
  queue_update(queue);

  strcpy(task->name,name);
  printf("Task name           : %s\n", task->name);
  sprintf(task->qname, "task%d", task->id);
  
  strcpy(task->cmd,script);
  printf("Script              : %s\n", task->cmd);
  
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    strcpy(task->cwd,cwd);
    strcat(task->cwd,"/");
    printf("Current working dir : %s\n", task->cwd);
  }
  
  task->nproc=atoi(nproc);
  printf("nproc               : %d\n", task->nproc);


  // retrive the current environment and store it in the task
  task->uid=pw->pw_uid;
  task->gid=pw->pw_gid;
  strcpy(task->logname,login);
  
  sprintf(task->env[0],"TERM=xterm");
  sprintf(task->env[1],"USER=%s",pw->pw_name);
  sprintf(task->env[2],"HOME=%s",pw->pw_dir);
  sprintf(task->env[3],"SHELL=%s",pw->pw_shell);
  sprintf(task->env[4],"LOGNAME=%s",pw->pw_name);
  sprintf(task->env[5],"PATH=/usr/bin:/bin:/opt/bin:/usr/local/bin");
  task->env[6]=0;

  task_setup(task);

  // output of environment
  printf("owner               : %s\n", task->logname);
  printf("env                 : %s\n", task->env[0]);
  printf("env                 : %s\n", task->env[1]);
  printf("env                 : %s\n", task->env[2]);
  printf("env                 : %s\n", task->env[3]);
  printf("env                 : %s\n", task->env[4]);
  printf("env                 : %s\n", task->env[5]);

  // write new teask to init_dir
  sprintf(fname, "%s/init/%s", root,task->qname);
  task_write(task, fname);

  // clean up
  task_bin(task);
  free(task);
  queue_stop(queue);
  free(queue);
}

/*
 * queue_cli
 */
void queue_cli()
{
  char* root=INSTALL_DIR;
 
  Queue *queue = malloc(sizeof(Queue));
  queue_init(queue,root);

  char queued_dir[1000];
  char running_dir[1000];

  strcpy(queued_dir,root);
  strcat(queued_dir,"/queued/");
  strcpy(running_dir,root);
  strcat(running_dir,"/running/");

  // Header
  printf("Queue Version %4.1f\n",queue_version(queue));
  printf("------------------\n");
  printf("nproc: %d\n\n",queue->NPROC);
  
  printf("%6s %8s %25s %8s %6s %8s %25s\n","ID","pid","Name","Status","proc","Dt/min","Date");
  printf("--------------------------------------------------------------------------------------------\n");

  // state: 0 ... initialization
  //        1 ... queued
  //        2 ... running
  add_tasks_to_queue(&queue->queued,queued_dir,1);
  add_tasks_to_queue(&queue->running,running_dir,2);
  
  // print all tasks, appling same format as above
  print_list(&queue->queued);
  print_list(&queue->running);
  
  // clean up
  queue_stop(queue);
  free(queue);
}


/*
 * list all currently init, queued and runnung tasks
 */
void list_queue()
{
  char* root=INSTALL_DIR;
 
  Queue *queue = malloc(sizeof(Queue));
  queue_init(queue,root);

  char init_dir[1000];
  char queued_dir[1000];
  char running_dir[1000];

  strcpy(init_dir,root);
  strcat(init_dir,"/init/");
  strcpy(queued_dir,root);
  strcat(queued_dir,"/queued/");
  strcpy(running_dir,root);
  strcat(running_dir,"/running/");


  printf("Queue Version %4.1f\n",queue_version(queue));
  printf("------------------\n\n");

  /* printf("This system has %d processors configured and " */
  /* 	 "%d processors available.\n\n", */
  /* 	 get_nprocs_conf(), get_nprocs()); */
  
  printf("  ID    Queue Name                        Status       proc\n");
  printf("-----------------------------------------------------------\n");

  list_tasks(init_dir,"init");
  list_tasks(queued_dir,"queued");
  list_tasks(running_dir,"running");

  // clean up
  queue_stop(queue);
  free(queue);
}



/*
 * check if new tasks are availabe in the init_dir
 * if so change state from INIT to QUEUED
 */
void retrive_tasks(Queue *queue,char* root)
{
  DIR *d;
  struct dirent *dir;
  
  Task * task;
  
  char buffer[2500];
  
  char fname[1000];
  
  char oldname[1000];
  char newname[1000];
  
  char init_dir[1000];
  char queued_dir[1000];

  strcpy(init_dir,root);
  strcat(init_dir,"/init/");
  
  strcpy(queued_dir,root);
  strcat(queued_dir,"/queued/");

  d = opendir(init_dir);
  
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if( strcmp(dir->d_name,"." )!=0 &&
	  strcmp(dir->d_name,"..")!=0    ) {
	sprintf(fname, "%s/init/%s", root,dir->d_name);
	
	task = task_read(fname);

	// Logging
	sprintf(buffer, "%d _r_ task reading %s\n",task->id,fname);
	printf(buffer);
	_logging(buffer);

	// set queued time
	task->time_queued=time(NULL);

	// push task to the queued list, entering the QUEUED state
	push_list(&queue->queued, task);

	// just copy
	strcpy(oldname,init_dir);
	strcat(oldname,task->qname);
	strcpy(newname,queued_dir);
	strcat(newname,task->qname);
	rename(oldname, newname);

	// update times
	task_write(task, newname);
      }
    }
    closedir(d);
  }
}

/*
 * list all taskts in submitted input dir and assign the given state
 */
void list_tasks(char * input,char * state)
{
  DIR *d;
  struct dirent *dir;
  
  Task * task;
   
  char fname[1000];  


  d = opendir(input);
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if( strcmp(dir->d_name,"." )!=0 &&
	  strcmp(dir->d_name,"..")!=0    ) {
	sprintf(fname, "%s%s", input,dir->d_name);

	// retrive task
	task = task_read(fname);

	// print informations
	printf("%4d    %.10s                           %.10s   %4d\n",task->id,task->name,state,task->nproc);

	// discard the task
	task_bin(task);
	free(task);
      }
    }
    closedir(d);
  }
  
}

/*
 * add all tasks being in the input dir to the queue list (qlist)
 * and assign given state
 */
void add_tasks_to_queue(list_t ** qlist, char * input,int state)
{
  DIR *d;
  struct dirent *dir;
 
  Task * task;
   
  char fname[1000];  

  d = opendir(input);
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if( strcmp(dir->d_name,"." )!=0 &&
	  strcmp(dir->d_name,"..")!=0    ) {
	sprintf(fname, "%s%s", input,dir->d_name);
	
	// retrive task
	task = task_read(fname);

	// assign state
	task->state=state;
	
	// discard the task
	push_list(qlist, task);
      }
    }
    closedir(d);
  }
}



/*
 * the queueing procedure, so called big loop
 * Uniting all routines
 */
void run_queue()
{
  char* root=INSTALL_DIR;
  
  int status;
  
  Task *temp;
  list_t *list_temp = NULL;

  char buffer[1000];
  
  char oldname[1000];
  char newname[1000];
  
  char queued_dir[1000];
  char running_dir[1000];
  char finished_dir[1000];

  pid_t fork_pid;

  int fd;
    
  Queue *queue = malloc(sizeof(Queue));
  queue_init(queue,root);

  // print informations about the Queue
  printf("Queue Version %4.1f\n",queue_version(queue));
  printf("------------------\n\n");

  printf("This system has %d processors configured and "
	 "%d processors available.\n\n",
	 queue->NPROC, get_nprocs());

  // Prepare all paths
  strcpy(queued_dir,root);
  strcat(queued_dir,"/queued/");
  
  strcpy(running_dir,root);
  strcat(running_dir,"/running/");
  
  strcpy(finished_dir,root);
  strcat(finished_dir,"/finished/");

    
  
  /*
   * if task is in RUNNING state
   * add up procs
   */
  add_tasks_to_queue(&queue->running,running_dir,2);
  if(queue->running) {
    list_temp=queue->running;
    while(list_temp) {
      temp=list_temp->task;
      list_temp=list_temp->next;
      queue->PROC+=temp->nproc;
      removeID_list(&queue->running,temp->id);
    }
  }

  
  // big loop
  while(1) {
    // INIT -> QUEUED
    retrive_tasks(queue,root);

    /*
     * if task is in QUEUED state
     * check if needed resources are available to start task
     * if so QUEUED -> RUNNING
     */
    /* printf("queue->NPROC          : %d\n",queue->NPROC); */
    /* printf("queue->PROC           : %d\n\n",queue->PROC); */
    
    if(queue->queued) {
      list_temp=queue->queued;
      while(list_temp) {
	// resources?
	if(queue->NPROC>=queue->PROC+list_temp->task->nproc) {  
	  temp=list_temp->task;
	  
	  // block resources
	  queue->PROC+=temp->nproc;

	  list_temp=list_temp->next;

	  // logging
	  sprintf(buffer, "%d _q_ task moved from queued to running (%d/%d)\n",temp->id,queue->PROC,queue->NPROC);
	  printf(buffer);
	  _logging(buffer);

	  // set running time
	  temp->time_running=time(NULL);
	  
	  // move task QUEUED -> RUNNING
	  moveID_list(&queue->queued,&queue->running,temp->id);

	  // fork off
	  fork_pid = fork();
	  if (fork_pid == 0) {
	    /*
	     * child
	     */

	    //to redirect standard input to a file
	    fd = open(temp->log, O_RDWR | O_CREAT, 00666);
	    dup2(fd, 1);
	    dup2(fd, 2);
	    close(fd);

	    // fix environment -> representing same state as when task was submitted
	    if(chdir(temp->cwd));
	    initgroups(temp->logname,temp->gid);
	    if(setgid(temp->gid));
	    if(setuid(temp->uid));

	    /*
	     * without environment
	     * execvp(temp->argv[0],temp->argv);
	     */
	    
	    // with environment
	    execve(temp->argv[0],temp->argv,temp->env);

	    // so long suckes!
	    return;
	  }
	  else {
	    // paranet
	    temp->pid=fork_pid;

	    // logging
	    sprintf(buffer, "%d _s_ task (pid %d) started (%d/%d)\n",temp->id,temp->pid,queue->PROC,queue->NPROC);
	    printf(buffer);
	    _logging(buffer);

	    // move file from QUEUED -> RUNNING
	    strcpy(oldname,queued_dir);
	    strcat(oldname,temp->qname);
	    strcpy(newname,running_dir);
	    strcat(newname,temp->qname);
	    status=rename(oldname, newname);
	    
	    // update times in file
	    task_write(temp, newname);
	  }
	}
	else {
	  // check next in line
	  list_temp=list_temp->next;
	}
      }
    }

    /*
     * if task is in RUNNING state
     * check if it is still runnung
     * otherwise move RUNNING -> FINISED
     */
    if(queue->running) {
      list_temp=queue->running;
      while(list_temp) {
	temp=list_temp->task;
	list_temp=list_temp->next;

	/*
	 * is task still alive?
	 */
	if(waitpid(temp->pid, &status, WNOHANG)>0) {
	  // nope sucker is gone

	  // free resources
	  queue->PROC-=temp->nproc;
	  
	  // logging
	  sprintf(buffer, "%d _f_ task (pid %d) finised (%d/%d)\n",temp->id,temp->pid,queue->PROC,queue->NPROC);
	  printf(buffer);
	  _logging(buffer);

	  // enter final FINISHED stage
	  temp->time_finished=time(NULL);
	  
	  // move file from running -> finished
	  strcpy(oldname,running_dir);
	  strcat(oldname,temp->qname);
	  strcpy(newname,finished_dir);
	  strcat(newname,temp->qname);
	  //printf("\t %s %s \n",oldname,newname);
	  status=rename(oldname, newname);

	  // update times in file
	  task_write(temp, newname);
	  
	  removeID_list(&queue->running,temp->id);
	}
      }
    }

    // time to sleep
    sleep(WAITTIME);
  }

  // clean up
  queue_stop(queue);
  free(queue);
}
