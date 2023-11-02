
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#define BUFFER 32

// By PJ Oschmann, Lukas Mccain, Anthony Kacki

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
    char program[BUFFER];
    
};

struct procMonitorParams {
  int pid;
  pthread_t* thread;
  struct processes* status;
};

struct procMonitorParams pmp[1] = { {} };

pthread_mutex_t lock;

// Strings for running and ready
char* running = "Running";
char* ready = "Ready";



// Monitor the child (The Babysitter)
void* procMonitor(void* arg) {
  pthread_mutex_lock(&lock);
  struct procMonitorParams* params = (struct procMonitorParams*) arg;
  int pid = params->pid;
  pthread_t* thread = params->thread;
  struct processes* statusStruct = params->status;
  pthread_mutex_unlock(&lock);

  int status = -99;
  waitpid(pid, &status,0);
  statusStruct->PID = 0;
  statusStruct->priority = 0;
  statusStruct->status = "\0";
  //statusStruct->program ="\0";
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

    // String set for current status
    char* statusToSet = "NOSTATUS";

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

        //printf("Program: %s\n", status[0].program);

        // if user types in "status" then display the background processes
        if(!strcmp(programAndPriority, "status\n")){
            printf("PID        Priority        Status        Program\n");
            for (int i=0; i < 5; i++) {
              if (status[i].PID != 0) {
                  // TODO: Implement priority and ready state

                  printf("%d     %d               %s        %s\n"
                  ,status[i].PID
                  , status[i].priority
                  , status[i].status
                  , status[i].program);

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
        int begin = 0;

        // Only create the new proc if the file name exists
        check = file_exists (name);
        if (check == 1) {
          int emptyNum = 0;

          // Anything in chart?
          for (int i=0; i<5; i++) {
            // Everything empty?
            if (status[i].PID == 0) {
              emptyNum++;
            }
          }
          // So it is empty? Just start the programs
          if (emptyNum == 5) {
            statusToSet = running;
            begin = 1;
          }
          // Something's there? Gotta compare
          else {
            begin = 0;
          }
          
          // TODO: There are probably bugs with priority
          // TODO: Gotta continue it!

          // First, see if any of same priority are going
          // TODO: Check if higher is going to determine start or not
          if (begin == 0) {
            for (int i=0; i<5; i++) {
              // First, see if any are equal that we can stop
              if (priorityNum == status[i].priority) {
                status[i].status = ready;
                statusToSet = running;
                kill(status[i].PID, SIGSTOP);
                printf("She's equal\n");
                begin = 1;
                break;
              }
            }
          }

          // No eq priority? See if my priority is less than something in chart
          if (begin == 0) {

            for (int i=0; i<5; i++) {
              // No dice? Find something smaller
              if (priorityNum < status[i].priority) {
                printf("She's less :(\n");
                statusToSet = ready;
                begin = 1;
                break;
              }
            }
          }

          // Next, see if my priority is higher than others
          if (begin == 0) {
            for (int i=0; i<5; i++) {
              if (priorityNum > status[i].priority) {
                printf("She's high af\n");
                status[i].status = ready;
                statusToSet = running;
                kill(status[i].PID, SIGSTOP);
                begin = 1;
                break;
              }
            }
          }



          // See where to run the process. If all 5 slots are taken,
          // don't run it.
          for (int i=0; i<5; i++) {

                // Slot found? Run.
                if (status[i].PID == 0) {
                  slot = i;
                  doRun = 1;
                  break;
                }

                // No open slots? Don't run.'
                else if (i == 4) {
                  doRun = 0;
                  printf("Too many processes are running. Please wait for one "
                    "to finish or kill one. \n");
                }
          }

          // If doRun is true, fork and execve.
          if (doRun) {

            pid = fork();

            //TODO: Set priorities (highest runs first), same priority, kill the
            // running proc and start the new one

            // Parent; hire the babysitter
            if(pid > 0){
                status[slot].PID = pid;
                status[slot].priority = priorityNum;
                status[slot].status = statusToSet;
                for (int i=0; i< BUFFER; i++) {
                  if (name[i] == '\0') {
                    break;
                  }
                  status[slot].program[i] = name[i];
                }
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
        else {
          printf("Error: Program not found.\n");
        }
    }
}


