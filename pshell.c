
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
};

struct procMonitorParams pmp[1] = { {} };

// Monitor the child (The Babysitter)
void* procMonitor(void* arg) {
  struct paramForBg* new = (struct paramForBg*) arg;

  //TODO: If desired, we can remove these variables before submission.
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
    
  
    //int procStatus = -99;
  
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
        //printf("\n[Program and Priority: %s]",programAndPriority);
        //printf("\n[Name: %s]", name);
        //printf("\n[Priority: %d]", priorityNum);

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
                    status[i].status = '\0';
                    status[i].program = '\0';
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
        
        // Only create the new proc if the file name exists
        check = file_exists (name);
        if (check == 1) {
          
          pid = fork();
          //TODO: Remove me!
          // The below test code shows the status of the program.
          int testProcStatus = -99;
          //printf("[Proc Status for %d is: %d]",pid,testProcStatus);
          // End test code
          
          //TODO: Set priorities (highest runs first), same priority, kill the
          // running proc and start the new one
           
          // Parent; hire the babysitter
          if(pid != 0){
              //printf("[I'm parent, Child's PID %d] ", pid);
              for (int i=0; i<5; i++) {
                if (status[i].PID == 0) {
                  pthread_create(&threads[i], NULL, procMonitor, pmp);
                  break;
                }
                else if (i == 5) {
                  printf("Too many processes are running. Please wait for one to \
                    finish or kill one. \n");
                }
              }
              continue;
          }
          // Child; replace image using execve
          else{
              char* const* argv;
              char* const* envp;
              execve(name, argv, envp);
          }
        }
    }
}


