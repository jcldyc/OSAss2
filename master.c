//Link to project https://mygateway.umsl.edu/bbcswebdav/pid-4181242-dt-content-rid-25566240_1/courses/UMSL-CMPSCI4760-002-12129-FS2017/cs4760Assignment2Fall2017Hauschild%281%29.pdf

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

void ctrlPlusC(int sig);
void cProcExec(int cIndex, int pIndex);

//struct used for the multiprocessor solution

enum state {			
 idle, want_in, in_cs 
};

typedef struct PalInfo{							
    char pList[50][256];
    int turn;
    enum state flag[19];
} palInfo;

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
	// references:  
	// https://www.youtube.com/watch?v=SMeDw2GDMsE
	// http://users.cs.cf.ac.uk/Dave.Marshall/C/node27.html
	// https://stackoverflow.com/questions/13334352/how-to-attach-an-array-of-strings-to-shared-memory-c
	
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
        perror("fork()");
        abort();
      } else if (pids[i] == 0) {
        cProcExec(i, pCount);
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

void cProcExec(int cIndex, int pIndex){

    char workingDir[256];
    getcwd(workingDir, sizeof(workingDir));
    char placeToExec[1024];
    strcpy(placeToExec, workingDir);
    strcat(placeToExec, "/palin");

    char index[10];
    snprintf(index, 10, "%d", cIndex);

    char palinIndex[10];
    snprintf(palinIndex,10,"%d", pIndex);

    char *args[]={"./palin", index, palinIndex};		//this passes the arguments needed to exec the child  process

    execvp("./palin", args);
    printf("Error while executing ./palin for child %d\n", cIndex+1);
    exit(1);
}






