/*
 * A linked list for tasks
 *
 * (c) christoph irrenfried, chi86
 *
 */

#include "List.h"

/*
 * Print all tasks of list head
 */
void print_list(list_t ** head,int verbose)
{
  list_t * current = *head;
  char state[19];
  double diff_t;
  
  char controlDict[1000];
  FILE * controlDFile;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  char* ret,ret_solver;
  /* int lineCount; */
  char tmp[2000],solver[2000];
  //int endTime,currentTime0,currentTime;
  float endTime,currentTime0,currentTime;
  int args;
  
  DIR *d;
  struct dirent *dir;

  clock_t time0,time_now;

  /* char Squeued[100], Srunning[100]; */
  
  while (current != NULL) {
    strcpy(state,current->task->state == 1 ? "\033[0;35mqueued\033[0m" : "\033[0;31mrunning\033[0m");

    time0=current->task->state == 1 ? current->task->time_queued : current->task->time_running;
    time_now=time(NULL);
    
    diff_t = difftime(time_now, time0)/60;
    
      
    printf("%6d \033[0;33m%8d\033[0m %25s %19s %6d %8.0f \033[0;32m%26s\033[0m",current->task->id,current->task->pid,current->task->name,state,current->task->nproc,diff_t,asctime(localtime(&time0)));

    if(verbose == 1) {
      printf("\t %25s\n",current->task->cwd);
    }

    // Check if the task is a OpenFoam simulation
    strcpy(controlDict,current->task->cwd);
    strcat(controlDict,"system/controlDict");
    /* printf("%42s %s\n","",controlDict); */
    controlDFile = fopen(controlDict,"r");
    if (controlDFile!=NULL) {
      // if file exists --> its a OF simulation

      // extract endTime from controlDict
      /* lineCount=0; */
      while ((read = getline(&line, &len, controlDFile)) != -1) {
	/* printf("Retrieved line of length %zu:\n", read); */
        //printf("%s", line);

	// ret=strstr(line,"application"); --> address pointer to entry
	if(strstr(line,"application")) {
	  if(!strstr(line,"//")) {
	    /* printf("%d : %p: %s\n",lineCount,ret,line); */
	    sscanf(line, "%s %s\n", tmp, solver);	    
	  }
	}

	/* printf("test %s\n",line); */
        
	if(strstr(line,"endTime")) {
	  if(!strstr(line,"//") && !strstr(line,"stopAt")) {
	    /* printf(" : %p: %s\n",ret,line); */
	    /* printf("%d : %p: %s\n",lineCount,ret,line); */
	    sscanf(line, "%s %f;\n", tmp, &endTime);	    
	  }
	}
	/* lineCount+=1; */
      }
      fclose(controlDFile);

      // printf("endtime %f\n",endTime);
 
      // remove ; at end
      solver[strlen(solver)-1] = '\0';

      // check if simulation already started
      strcpy(controlDict,current->task->cwd);
      strcat(controlDict,"log.");
      strcat(controlDict,solver);
      /* printf("%42s %s\n","",controlDict); */
      controlDFile = fopen(controlDict,"r");
       
      if (controlDFile!=NULL) {

	
	////////
	////////
	// retrive current time from log file
	////////
	////////
	currentTime=0;
	// extract currenttime from log.$Solver$ file
	while ((read = getline(&line, &len, controlDFile)) != -1) {
	  // ret=strstr(line,"application"); --> address pointer to entry
	  
	  if(strstr(line,"Iteration =") || strstr(line,"Time =") && !strstr(line,"ExecutionTime =") ) {
	    /* printf("c: %s\n",line); */
	    sscanf(line, "%s = %f\n", tmp, &currentTime);	    
	  }
	  // foam-ext
	  if(strstr(line,"Time iteration:")) {
	    //printf("c: %s\n",line);
	    sscanf(line, "Time iteration: %f\n", &currentTime);   
	  }
	}
	/* printf("endtime %f\n",endTime); */
	/* printf("currentTime %f\n",currentTime); */
	
	////////
	////////
	// check if time folder is already here */
	////////
	////////
	/* fclose(controlDFile); */
	
	/* // get currentTime from processor folder */
	/* currentTime=0; */
	/* strcpy(controlDict,current->task->cwd); */
	/* if(current->task->nproc>1) strcat(controlDict,"processor0/"); */
	/* d = opendir(controlDict); */

	     
	/* /\* printf("test %s\n",controlDict); *\/ */
      
	
	/* while((dir = readdir(d)) != NULL) { */
	/*   if( strcmp(dir->d_name,"." )!=0 && */
	/*       strcmp(dir->d_name,"..")!=0    ) { */
	/*     /\* printf("%s\n",dir->d_name); *\/ */
	/*     if(!strstr(dir->d_name,"constant") && !strstr(line,"0")) { */
	/*       sscanf(dir->d_name, "%f",&currentTime0); */
	/*       currentTime=MAX(currentTime,currentTime0); */
	/*     } */
	/*   } */
	/* } */
	/* printf("%s %f %f %s\n",solver,currentTime,endTime,controlDict); */

	
	printf("%42s simulation progress=\033[0;31m%.2f \%\033[0m\n","",(100.0*currentTime/endTime));
      }
    }
    current=current->next;
  }

  

    
  
}

/*
 * Push task to list head
 */
void push_list(list_t ** head, Task *task)
{
  list_t * temp = malloc(sizeof(list_t));
  list_t * last = *head;
  
  temp->task = task;
  temp->next = NULL;
  
  // If the Linked List is empty, then make the new node head
  if (*head == NULL)
    {
      *head = temp;
      return;
    }
  // go to last node
  while (last->next != NULL)
    last = last->next;
  
  // Change the next in line
  last->next = temp;
  return;
}

/*
 * Pop task from list head
 */
Task * pop_list(list_t ** head)
{
  Task * task;
  list_t * current = *head;
  list_t * next = current->next;
  
  task=current->task;

  free(current);

  *head=next;
  
  return task;
}

/*
 * Remove task with id from list head
 */
void removeID_list(list_t ** head, int id)
{
  Task * task;
  list_t * current = *head;
  list_t * temp = NULL;

  // being first in line
  if(current->task->id==id) {
    *head=current->next;
    task=pop_list(&current);
    task_bin(task);
    free(task);
    return;
  }

  // further up the list
  while(current) {
    if(current->next->task->id==id) {
      temp=current->next->next;
      task_bin(current->next->task);
      free(current->next->task);
      free(current->next);
      current->next=temp;
      return;
    }
    current=current->next;
  }
}


/*
 * Move task id from list head to list sink
 */
void moveID_list(list_t ** head,list_t ** sink, int id)
{

  list_t * current = *head;
  list_t * temp = NULL;

  // being first in line
  if(current->task->id==id) {
    *head=current->next;
    push_list(sink,pop_list(&current));
    return;
  }

  // further up the list
  while(current) {
    if(current->next->task->id==id) {
      temp=current->next->next;
      push_list(sink,current->next->task);
      free(current->next);
      current->next=temp;
      return;
    }
    current=current->next;
  }
}
