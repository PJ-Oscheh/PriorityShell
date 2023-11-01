
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#define BUFFER 32


// create a shell that allows a user to run programs and specify priority of
// them

// highest priority process runs until it completes or a new higher priority
// process is entered (priorities are 1-3)

// all processes will run in the background

// only one process runs in the background at a time

// if a user enters a new process with a higher priority then stop the current
// and run that one

// use the kill() system call to send the appropriate signal to process
// (SIGSTOP and SIGCONT)

// if user types in "status" all the processes should list out

// create a struct for entries to the table, an array of 5

// struct to hold all of the data in the status table
struct processes{
    int PID;
    int priority;
    char* status;
    char* program;
    int procStatus;
    
};

struct procMonitorParams {
  int pid;
  pthread_t* thread;
  struct processes* status;
};

struct procMonitorParams pmp[1] = { {} };

pthread_mutex_t lock;

// Monitor the child (The Babysitter)
void* procMonitor(void* arg) {
  pthread_mutex_lock(&lock);
  struct procMonitorParams* params = (struct procMonitorParams*) arg;
  int pid = params->pid;
  pthread_t* thread = params->thread;
  struct processes* statusStruct = params->status;
  pthread_mutex_unlock(&lock);

  int status = -99;
  printf("Process %d is running\n",params->pid);
  waitpid(pid, &status,0);
  statusStruct->PID = 0;
  pthread_detach(*thread);
  return NULL;
}

//Return 1 if file exists, 0 if not
int file_exists(const char* fileName) {
  
  FILE* filep = fopen(fileName, "r");
  int fileExists = 0;
  if (filep != NULL) {
    fileExists = 1;
    fclose(filep);
  }
  return fileExists;
}

int main(void) {
    // holds the input from the shell
    char programAndPriority[BUFFER];

    // holds the name of the program
    char name[BUFFER];

    // holds the priority of that program
    char priority = 0;

    struct processes status[5] = { {}, {}, {}, {}, {} };

    // the threads
    pthread_t threads[5];
    
  
    int priorityNum = -99;
  
    for (int i=0; i<5; i++) {
      status[i].procStatus = -99;
    }
  
    // start shell
    while(1) {
        
      
        for (int i=0; i< BUFFER; i++) {
          programAndPriority[i] = '\0';
          name[i] = '\0';
        }
        priority = '\0';
        priorityNum = 0;
        
        // Get program name and priority from user
        printf("pshell:");
        fgets(programAndPriority, BUFFER, stdin);
        // parses the name of the program and the priority number out of the
        // user input
        for (int i = 0; programAndPriority[i] != '\0'; i++) {
            
            if (programAndPriority[i] == ' ') {
                name[i] = '\0';
                // converts priority to an int
                priority = programAndPriority[i + 1];
              
                // Set integer for priority
                priorityNum = atoi(&priority);
                
                if (programAndPriority[i+2] != '\0') {
                  programAndPriority[i+2] = '\0';
                }
                break;
            }

            name[i] = programAndPriority[i];
        }

        // if user types 0 then quit
        if (programAndPriority[0] == '0')
            exit(0);


        // if user types in "status" then display the background processes
        if(!strcmp(programAndPriority, "status\n")){
            
            printf("PID        Priority        Status        Program\n");
            for (int i=0; i < 5; i++) {
              
              if (status[i].PID != 0) {
                  waitpid(status[i].PID, &status[i].procStatus, WNOHANG);
                  
                  // If return -99, it's running or ready
                  // If return 0, it's not running and should be removed from
                  //  the list.
                  
                  if (status[i].procStatus == -99) {
                    
                    // TODO: Implement priority and ready state
                    status[i].status = "Running";
                    
                    printf("%d     %d               %s        %s\n"
                    ,status[i].PID
                    , status[i].priority
                    , status[i].status
                    , status[i].program);
                  }
                  else if (status[i].procStatus == 0) {
                    status[i].PID = 0;
                    status[i].priority = 0;
                    status[i].status = "\0";
                    status[i].program = "\0";
                    status[i].procStatus = -99;
                  }
                  
                } 

            }
            
            continue;
        }

        // check if the priority is 1-3
        if (!(priorityNum == 1 | priorityNum == 2 | priorityNum == 3)) {
            printf("Error: Invalid priority\n");
            //continue;
        }

        //*********************************************************************
        // Now we are going to create the new process
        
        int pid = -99;
        int check = -99;
        int doRun = 0;
        int slot = -99;
        // Only create the new proc if the file name exists
        check = file_exists (name);
        if (check == 1) {
          
          // See where to run the process. If all 5 slots are taken,
          // don't run it.
          for (int i=0; i<5; i++) {
                if (status[i].PID == 0) {
                  slot = i;
                  printf("Using thread %d\n",i);
                  doRun = 1;
                  break;
                }
                else if (i == 4) {
                  doRun = 0;
                  printf("Too many processes are running. Please wait for one "
                    "to finish or kill one. \n");
                }
          }
          if (doRun) {
            pid = fork();

            //TODO: Set priorities (highest runs first), same priority, kill the
            // running proc and start the new one

            // Parent; hire the babysitter
            if(pid != 0){
                printf("Once again, using thread %d\n", slot);
                status[slot].PID = pid;
                pmp[0].pid = pid;
                pmp[0].thread = &threads[slot];
                pmp[0].status = &status[slot];
                pthread_create(&threads[slot], NULL, procMonitor, pmp);
                continue;
            }


            else{

                char* const* argv;
                char* const* envp;
                execve(name, argv, envp);


            }
          }
        }
    }
}


