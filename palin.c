#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

const int isPalindrome(char * palindromeString);
const void writeToFile(int isAPalindrome);
void exitfuncCtrlC(int sig);
void exitfunc(int sig);
void exitfuncAlarm(int sig);



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




    if (signal(SIGUSR1, exitfunc) == SIG_ERR) {
        printf("SIGUSR1 error\n");
        exit(1);
    }
    if (signal(SIGINT, exitfuncCtrlC) == SIG_ERR) {
        printf("SIGINT error\n");
        exit(1);
    }
    if (signal(SIGALRM, exitfuncAlarm) == SIG_ERR){
        printf("SIGALRM error\n");
    }

    

    int id;
    int key = 9929;



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
    
    for (int i = 0; i < 5; ++i)
    {
        
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
            time(&timer);
            tm_info = localtime(&timer);

            strftime(wantInTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            fprintf(stderr, "\t%s\tprocess: %d\twants to enter the critical section.\n", wantInTime, procNum);

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
        int randomNumber = rand()%3;
        sleep(randomNumber);



        FILE * filePtr;

        if(isAPalindrome){
            filePtr = fopen("palin.out","a");

        }
        else{
            filePtr = fopen("nopalin.out","a");
        }

        fprintf(filePtr, "%ld %d %s\n", (long)getpid(), currentpalNum-n+1, possiblePalindrome);

        fclose(filePtr);


        time(&timer);
        tm_info = localtime(&timer);
        srand(time(NULL));
        randomNumber = rand()%3;
        sleep(randomNumber);

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


const int isPalindrome(char * palindromeString){					//Palindrome website reference: http://www.geeksforgeeks.org/c-program-check-given-string-palindrome/


    // Start from leftmost and rightmost corners of str
    int l = 0;
    int h = strlen(palindromeString);
 
    // Keep comparing characters while they are same
    while (h > l)
    {
        if (palindromeString!= palindromeString[h--])
        {
            return 0;				//returns a 0 if it is NOT a palindrome
        }
    }
    return 1;		


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

void exitfuncAlarm(int sig){

   
    fprintf( stderr, "Alarm Signal from parent. Detaching memory and killing self\n");


    shmdt(shmPtr);
   

    exit(1);
}
