/*
 * Progress report for the queue list
 *
 * (c) christoph irrenfried, chi86
 *
 */

#include "Progress.h"

void progress_openfoam(char* cwd) {

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

  strcpy(controlDict,cwd);
  strcat(controlDict,"system/controlDict");
  /* printf("%42s %s\n","",controlDict); */
  controlDFile = fopen(controlDict,"r");
  
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
  strcpy(controlDict,cwd);
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
