
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
};

struct paramForBg {
  char* name;
};

struct paramForBg theParams[1] = { {} };

// creates a seperate processes using execev
void* bgProc(void* arg) {
  struct paramForBg* new = (struct paramForBg*) arg;

  //TODO: If desired, we can remove these variables before submission.
  char* const* argv;
  char* const* envp;
  execve(new->name, argv, envp);
  return NULL;
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
    
  
    int procStatus = -99;
  
    // start shell
    while(1) {
        // Get program name and priority from user

        printf("pshell:");
        fgets(programAndPriority, BUFFER, stdin);

        // if user types 0 then quit
        if (programAndPriority[0] == '0')
            exit(0);


        // if user types in "status" then display the background processes
        if(!strcmp(programAndPriority, "status\n")){
            
            printf("PID        Priority        Status        Program\n");
            for (int i=0; i < 5; i++) {
              
              if (status[i].PID != 0) {
                  waitpid(status[0].PID, &procStatus, WNOHANG);
                  // If return -99, it's running or ready
                  // If return 0, it's not running and should be removed from
                  //  the list.
                  
                  if (procStatus == -99) {
                    
                    // TODO: Implement priority and ready state
                    status[i].status = "Running";
                    
                    printf("%d     %d               %s        %s\n"
                    ,status[i].PID
                    , status[i].priority
                    , status[i].status
                    , status[i].program);
                  }
                  else if (procStatus == 0) {
                    status[i].PID = 0;
                    status[i].priority = 0;
                    status[i].status = NULL;
                    status[i].program = NULL;
                  }
                  
                } 

            }
            
            continue;
        }


        // parses the name of the program and the priority number out of the
        // user input
        for (int i = 0; programAndPriority[i] != '\0'; i++) {
            if (programAndPriority[i] == ' ') {
                name[i] = '\0';
                priority = programAndPriority[i + 1];
                break;
            }

            name[i] = programAndPriority[i];
        }



        // converts priority to an int
        int priorityNum = atoi(&priority);

        // check if the priority is 1-3
        if (!(priorityNum == 1 | priorityNum == 2 | priorityNum == 3)) {
            printf("Error: Invalid priority\n");
            continue;
        }

        //*********************************************************************
        // Now we are going to create the new process

        int pid = fork();


        //TODO: Set priorities (highest runs first), same priority, kill the
        // running proc and start the new one

        // if we're in the parent then we're in the shell still so just continue
        if(pid != 0){
            for (int i=0; i<5; i++) {
              if (status[i].PID == 0) {
                status[i].PID = pid;
                break;
              }
            }
            continue;
        }
        // else we are in the child so we should create a new process with execve
        else{
            // find an empty space in the struct
            theParams[0].name = name;
            for (int i=0; i< 5; i++) {
              if (status[i].PID == 0) {
                pthread_create (&threads[i], NULL, bgProc, theParams);
              }
              else if (i==5) {
                printf("Too many processes are running. Please wait for one to \
                  finish or kill one.\n");
              }
            }

              for (int i=0; i<5; i++) {
                pthread_join(threads[i], NULL);
              }

              // After the program finishes running on its thread, exit so the
              // child doesn't continue in the while loop, making another
              // pshell prompt
              exit(0);
        }
    }
}


