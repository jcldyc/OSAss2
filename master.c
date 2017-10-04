#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>

//prototypes

void childProcessing(int childIndex, int palindromeIndex);
void ctrlPlusC(int sig);

typedef struct PalInfo{							
    char pList[50][256];
    int turn;
    enum state flag[][19];
} palInfo;


//struct used for the multiprocessor solution

enum state {			
 idle, want_in, in_cs 
};

palInfo shm;
palInfo *shmPtr;
pid_t pids[19];
int n = 19;
int id;


int main(int argc, char *argv[]){
    int i;
	int pCount = 0;			//sets the process count to 0 at the beginning
    int key = 3699;			//this  is the key used to identify the shared memory 
	 shmPtr = &shm;			//points the shared memory pointer to teh address in shared memory
	
	//Checks to make sure there is a file argument or if there is too many arguments
	
	if(argc < 2){
		printf("./master must be followed with a file. \n");
		exit(1);
	}else if(argc > 2){
		printf("Too many arguments. \n");
	}
	
	//Creates the files palin.out and nopalin.out and then closes them to use later
	
	FILE* file_ptr = fopen("palin.out", "w");
    fclose(file_ptr);
    file_ptr = fopen("nopalin.out", "w");
    fclose(file_ptr);
	
	//Opens the the file passed in as an argument: argv[1]
	
	FILE* file = fopen(argv[1], "r");
	
	//Sets each process id array position to -1
    for (i = 0; i < 19; ++i){
        pids[i] = -1;
    }
	
	//checks for user to hit Ctrl + C to exit 
	
    if (signal(SIGINT, ctrlPlusC) == SIG_ERR){			
        printf("SIGINT error\n");
        exit(1);
    }

    //Reserving memory.  Set's the permissions with  the 0666 argument
	
	id = shmget(key,sizeof(shm), IPC_CREAT | 0666);
    if (id < 0){
        perror("SHMGET");
        exit(1);
    }
	
	shmPtr = shmat(id, NULL, 0);
    if(shmPtr == (palInfo *) -1){
        perror("SHMAT");
        exit(1);
    }


    //Begin reading from the file with fget and put  into a char array
    char line[256];
    char endChar = '\0';
    i = 0;
    while (fgets(line, sizeof(line), file) && i<50) { 
        sprintf(shmPtr->pList[i], "%s",line);
        i++;
    }
    fclose(file);

    shmPtr->turn = 0;
    for (i = 0; i < 19; ++i){
            shmPtr->flag[i] = idle;			//sets that p's flag toidle which is used for multi processes 
    }

    // Create 19 processes from master/main process
	
    for (i = 0; i < 19; i++) {
      if ((pids[i] = fork()) < 0) {
        perror("fork");
        abort();
      } else if (pids[i] == 0) {
        childProcessing(i, pCount);
        printf("There was a problem creating the child.\n");
        exit(0);
      }
      pCount++;
    }

    // Used to wait and see when the children are done and exited,  Decrements n each time it exits the  for and  if loop
	
    int status;
    pid_t pid;
    while (n > 0) {
        pid = wait(&status);
        for (i = 0; i < 19; ++i){
            if (pids[i] == pid){
                pids[i] = -1;
            }
        }
        n--;
    }

    printf("Child processes have finished.\n");
    shmdt(shmPtr);					//must detach the shared memory
    shmctl(id, IPC_RMID, NULL);		//setting the control of the shared memory using the variable id = shmget(key,sizeof(shm), IPC_CREAT | 0666);
    return 0;
}

//used to quit the running program when the user presses Ctrl + C

void ctrlPlusC(int sig){
    fprintf( stderr, "Ctrl + C:  GO CRAZY!\n");
    shmdt(shmPtr);					//detach share mem
    shmctl(id, IPC_RMID, NULL);
    exit(1);
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






