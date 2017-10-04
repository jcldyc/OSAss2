#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

const int isPalindrome(char * palindromeString);
void exitfuncCtrlC(int sig);
void exitfunc(int sig);

enum state {
 idle, want_in, in_cs 
};

typedef struct Data{
    char palindromeList[50][256];
    int turn;
    enum state flag[19];
} SharedData;

SharedData shm;
SharedData *shmPtr;

int main(int argc, char *argv[]){
	
	int id;
    int key = 3699;
	shmPtr = &shm;

    if (signal(SIGUSR1, exitfunc) == SIG_ERR) {
        printf("SIGUSR1 error\n");
        exit(1);
    }
    if (signal(SIGINT, exitfuncCtrlC) == SIG_ERR) {
        printf("SIGINT error\n");
        exit(1);
    }
    
    //reserve space for struct and attach to pointer
    if ((id = shmget(key,sizeof(shm), IPC_CREAT | 0666)) < 0){
        perror("SHMGET");
        exit(1);
    }

    if((shmPtr = shmat(id, NULL, 0)) == (SharedData *) -1){
        perror("SHMAT");
        exit(1);
    }


    //Takes the arguments passed into ./palin and converts them from strings to ints
    int procNum = atoi(argv[1]);;
    int palNum = atoi(argv[2]); 

    time_t timer;
    struct tm* tm_info;

    //code to enter critical section
    int j;
    int n = 19;
    int currentpalNum = palNum;
    int isAPalindrome;
    
    for (int i = 0; i < 5; ++i){
        
        char possiblePalindrome[256];
        char startTime[26];
        char endTime[26];
        char wantInTime[26];

        if (currentpalNum < 50){
            strncpy(possiblePalindrome, shmPtr->palindromeList[currentpalNum],256);
            strtok(possiblePalindrome, "\n");
            currentpalNum = currentpalNum + n;
        }
        else{
            return 0;
        }

        //check if it is a palindrome
        if(isPalindrome(possiblePalindrome) != 0){
            isAPalindrome = 1;

        }
        else{
            isAPalindrome = 0;
        }
    
        do{
            /* time(&timer);
            tm_info = localtime(&timer);
            strftime(wantInTime, 26, "%H:%M:%S", tm_info); */
			time_t now;
			time(&now);
 
			printf("%s", ctime(&now));
			
            

            shmPtr->flag[procNum] = want_in; // Raise my flag
            j = shmPtr->turn; // Set local variable
            while ( j != procNum )                                            //while j != the process' constant
                j = ( shmPtr->flag[j] != idle ) ? shmPtr->turn : ( j + 1 ) % n;         //if the current process whose turn it is idle set j to (j+1) %n

            // Declare intention to enter critical section

            shmPtr->flag[procNum] = in_cs;

            // Check that no one else is in critical section

            for ( j = 0; j < n; j++ ){
                if ( ( j != procNum ) && ( shmPtr->flag[j] == in_cs ) ){
                    break;
                }
            }
        } while (( j < n ) || ( shmPtr->turn != procNum && shmPtr->flag[shmPtr->turn] != idle ));

        // Assign turn to self and enter critical section

        shmPtr->turn = procNum;      

        time(&timer);
        tm_info = localtime(&timer);
        strftime(startTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		
        fprintf(stderr, "\t%s\tprocess: %d\tentering critical section.\n", startTime, procNum);

        //critical_section
        srand(time(NULL));
        int rN = rand()%3;
        sleep(rN);

        FILE * filePtr;

        if(isAPalindrome){
            filePtr = fopen("palin.out","a");

        }
        else{
            filePtr = fopen("nopalin.out","a");
        }
        fprintf(filePtr, "%ld %d %s\n", (long)getpid(), currentpalNum-n+1, possiblePalindrome);
        fclose(filePtr);

  
        rN = rand()%3;
        sleep(rN);

        time(&timer);
        tm_info = localtime(&timer);
        strftime(endTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(stderr, "\t%s\tprocess: %d\texiting critical section.\n", endTime, procNum);
      
        j = (shmPtr->turn + 1) % n;
        while (shmPtr->flag[j] == idle)
            j = (j + 1) % n;

        // Assign turn to next waiting process; change own flag to idle

        shmPtr->turn = j;
        shmPtr->flag[procNum] = idle;
        
        //done in crit section
    }
    
    shmdt(shmPtr);
    return 0;
}

	//function that checks if the string is a palindrome
	//returns 1 or 2; 1= true & 2 = false
	//ref:  http://www.geeksforgeeks.org/c-program-check-given-string-palindrome/

	const int isPalindrome(char *str){
    // Start from leftmost and rightmost corners of str
    int l = 0;
    int h = strlen(str) - 1;
 
    // Keep comparing characters while they are same
    while (h > l)
    {
        if (str[l++] != str[h--])
        {
            return 0;				//NOT a pal
        }
    }
	return 1;						//IS a pal
}


//detaches shared memory 
void exitfunc(int sig){	
    shmdt(shmPtr);
    exit(1);
}


void exitfuncCtrlC(int sig){

    fprintf( stderr, "Child %ld is dying from parent control c\n", (long)getpid());
    shmdt(shmPtr);
    exit(1);
}


