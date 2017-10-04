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
    int key = 6644;



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


    //string to int
    int processIndex = atoi(argv[1]);;
    int palindromeIndex = atoi(argv[2]); 

    time_t timer;
    struct tm* tm_info;


    


    //code to enter critical section
    int j;
    int n = 19;
    int currentPalindromeIndex = palindromeIndex;
    int isAPalindrome;
    
    for (int i = 0; i < 5; ++i)
    {
        
        char myString[256];
        char startTime[26];
        char endTime[26];
        char wantInTime[26];

    

        if (currentPalindromeIndex < 50){
            strncpy(myString, shmPtr->palindromeList[currentPalindromeIndex],256);
            strtok(myString, "\n");
            currentPalindromeIndex = currentPalindromeIndex + n;
        }
        else{
            return 0;
        }



        

        //check if it is a palindrome
        if(isPalindrome(myString) != 0){
            isAPalindrome = 1;

        }
        else{
            isAPalindrome = 0;
        }
    
        do{
            time(&timer);
            tm_info = localtime(&timer);

            strftime(wantInTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            fprintf(stderr, "\t%s\tprocess: %d\twants to enter the critical section.\n", wantInTime, processIndex);

            shmPtr->flag[processIndex] = want_in; // Raise my flag
            j = shmPtr->turn; // Set local variable
            while ( j != processIndex )                                            //while j != the process' constant
                j = ( shmPtr->flag[j] != idle ) ? shmPtr->turn : ( j + 1 ) % n;         //if the current process whose turn it is idle set j to (j+1) %n

            // Declare intention to enter critical section


            shmPtr->flag[processIndex] = in_cs;



            // Check that no one else is in critical section

            for ( j = 0; j < n; j++ ){
                if ( ( j != processIndex ) && ( shmPtr->flag[j] == in_cs ) ){
                    break;
                }
            }
        } while (( j < n ) || ( shmPtr->turn != processIndex && shmPtr->flag[shmPtr->turn] != idle ));

        // Assign turn to self and enter critical section

        shmPtr->turn = processIndex;

        

        time(&timer);
        tm_info = localtime(&timer);

        strftime(startTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(stderr, "\t%s\tprocess: %d\tentering critical section.\n", startTime, processIndex);

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

        fprintf(filePtr, "%ld %d %s\n", (long)getpid(), currentPalindromeIndex-n+1, myString);

        fclose(filePtr);


        time(&timer);
        tm_info = localtime(&timer);
        srand(time(NULL));
        randomNumber = rand()%3;
        sleep(randomNumber);

        time(&timer);
        tm_info = localtime(&timer);
        strftime(endTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(stderr, "\t%s\tprocess: %d\texiting critical section.\n", endTime, processIndex);

        
        j = (shmPtr->turn + 1) % n;
        while (shmPtr->flag[j] == idle)
            j = (j + 1) % n;

        // Assign turn to next waiting process; change own flag to idle

        shmPtr->turn = j;
        shmPtr->flag[processIndex] = idle;
        
        //done in crit section

    }
    






   

    shmdt(shmPtr);
    return 0;
}


const int isPalindrome(char * palindromeString){


    int stringLength = strlen(palindromeString);

    char editedString[stringLength+1]; 
    //strcpy(editedString, palindromeString); 

    int j = 0;
    for(int i = 0; i < stringLength ;i++){
        if (isalnum(palindromeString[i])){
            editedString[j] = tolower(palindromeString[i]);
            j++;
        }
    }
    //null terminate new string
    editedString[j] = '\0';


    int editedStringLength = strlen(editedString);


    char editedStringReversed[editedStringLength];

    j = editedStringLength - 1;
    for (int i = 0; i < editedStringLength; ++i){
        editedStringReversed[i] = editedString[j];
        j--;
    }
    editedStringReversed[editedStringLength] = '\0';



    if (strcmp(editedString, editedStringReversed) == 0){
            return 1;
    }
    else{
            return 0;
    }    



}



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