#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>


void childProcessing(int childIndex, int palindromeIndex);

void exitfuncCtrlC(int sig);
void exitfuncAlarm(int sig);

enum state {
 idle, want_in, in_cs 
};

typedef struct Data{
    char palindromeList[50][256];
    int turn;
    enum state flag[19];
} SharedData;


pid_t pids[19];
int n = 19;
int id;
SharedData shm;
SharedData *shmPtr;

int main(int argc, char *argv[]){
    int i;
	
	if(argc !=2){
		printf("./master must be followed with a file. \n");
		exit(1);
	}
    for (i = 0; i < 19; ++i)
    {
        pids[i] = -1;
    }

    int configurableTime = 60;

    if (signal(SIGINT, exitfuncCtrlC) == SIG_ERR) {
        printf("SIGINT error\n");
        exit(1);
    }
    if (signal(SIGALRM, exitfuncAlarm) == SIG_ERR){
        printf("SIGALRM error\n");
    }


    alarm(configurableTime);
    
    int processesRunning = 0;
    
    int palindromeCount = 0;

    int key = 9929;

    FILE* file_ptr = fopen("palin.out", "w");
    fclose(file_ptr);
    file_ptr = fopen("nopalin.out", "w");
    fclose(file_ptr);


    shmPtr = &shm;

    //reserve space for struct and attach to pointer
    if ((id = shmget(key,sizeof(shm), IPC_CREAT | 0666)) < 0)
    {
        perror("SHMGET");
        exit(1);
    }

    if((shmPtr = shmat(id, NULL, 0)) == (SharedData *) -1)
    {
        perror("SHMAT");
        exit(1);
    }

    //attach file
    FILE* file = fopen(argv[1], "r");


    //read file into array
    char line[256];
    char endChar = '\0';
    i = 0;
    while (fgets(line, sizeof(line), file) && i<50) { 
        sprintf(shmPtr->palindromeList[i], "%s",line);
        i++;
    }
    fclose(file);

    shmPtr->turn = 0;
    for (i = 0; i < 19; ++i){
            shmPtr->flag[i] = idle;
    }

    // Spawn 19 children
    for (i = 0; i < 19; i++) {
      if ((pids[i] = fork()) < 0) {
        perror("fork");
        abort();
      } else if (pids[i] == 0) {
        //printf("forking process number: %d\n",i+1);
        childProcessing(i, palindromeCount);
        printf("error spawning child\n");
        exit(0);
      }
      palindromeCount++;
    }

    //Wait for children to exit.
    int status;
    pid_t pid;
    while (n > 0) {

        pid = wait(&status);

        for (i = 0; i < 19; ++i)
        {
            if (pids[i] == pid){
                pids[i] = -1;
            }
        }
        //printf("PID %ld exited with status %d.\n", (long)pid, status);
        n--;
    }

    //after all the processes have ran
    printf("All child processes have ran and exited\n");
    shmdt(shmPtr);
    shmctl(id, IPC_RMID, NULL);
    return 0;
}

void childProcessing(int childIndex, int palindromeIndex){


//get current working directory
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    //printf("Current working dir: %s\n", cwd);
    char executableLocation[1024];
    strcpy(executableLocation, cwd);
    strcat(executableLocation, "/palin");
    //printf("%s\n",  executableLocation);


    //int to string
    char index[10];
    snprintf(index, 10, "%d", childIndex);

    char palinIndex[10];
    snprintf(palinIndex,10,"%d", palindromeIndex);

    //set passed arguments. ./palin childIndex palindromeIndex
    char *args[]={"./palin", index, palinIndex};

    //printf("executing palin %d for pid %ld\n", childIndex + 1, (long)getpid());
    execvp("./palin", args);
    printf("Error while executing ./palin for child %d\n", childIndex+1);
    exit(127); //error out
}

void exitfuncAlarm(int sig){

    fprintf( stderr, "Too much time elapsed. Killing children, if any, and exiting program.\n");

    int i;
    for (i = 0; i < 19; ++i){
        if (pids[i] != -1){
            if (kill(pids[i], SIGUSR1) >= 0){
                fprintf(stderr, "killed child process %d\n", i+1);
            }

        }
    }
	
    shmdt(shmPtr);
    shmctl(id, IPC_RMID, NULL);

    exit(1);
}

void exitfuncCtrlC(int sig){


    sleep(1);
    fprintf( stderr, "CTRL C caught. Killing children and exiting program.\n");

    shmdt(shmPtr);
    shmctl(id, IPC_RMID, NULL);

    exit(1);
}




