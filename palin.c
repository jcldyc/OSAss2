#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

const int isPalindrome(char *inputString, int leftIndex, int rightIndex);
void printTime();
void exitfuncCtrlC(int sig);
void exitfunc(int sig);

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

int main(int argc, char *argv[]){
	
	int id;
    int pNum = atoi(argv[1]);;
    int thisPalindromeNumber = atoi(argv[2]); 
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
    
    
	//Shared memory:   Set's the permissions with  the 0666 argument
	// references:  
	// https://www.youtube.com/watch?v=SMeDw2GDMsE
	// http://users.cs.cf.ac.uk/Dave.Marshall/C/node27.html
	// https://stackoverflow.com/questions/13334352/how-to-attach-an-array-of-strings-to-shared-memory-c
	
    if ((id = shmget(key,sizeof(shm), IPC_CREAT | 0666)) < 0){
        perror("SHMGET");
        exit(1);
    }

    if((shmPtr = shmat(id, NULL, 0)) == (palInfo *) -1){
        perror("SHMAT");
        exit(1);
    }


    

    time_t timer;
    struct tm* tm_info;

    //code to enter critical section
    int j;
    int n = 19;
    int isAPalindrome;
    
    for (int i = 0; i < 5; ++i){
        
        char possiblePalindrome[256];
		

        if (thisPalindromeNumber < 50){
            strncpy(possiblePalindrome, shmPtr->pList[thisPalindromeNumber],256);
            strtok(possiblePalindrome, "\n");
            thisPalindromeNumber = thisPalindromeNumber + n;
        }
        else{
            return 0;
        }

       /*  //check if it is a palindrome
        if(isPalindrome(possiblePalindrome) != 0){
            isAPalindrome = 1;

        }
        else{
            isAPalindrome = 0;
        } */
    
        do{
            
			//Get's the time and outputs it when the  process is tyring to get into the CS
			//reference:  https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
			
			time_t rawtime;
			struct tm *info;
			char buffer[80];
			time( &rawtime );
			info = localtime( &rawtime );
			strftime(buffer,80,"%x - %I:%M:%S%p", info);
			
			//printTime();
            fprintf(stderr, "\t| %s | \t process: %d\t | Trying to enter Critical Section |\n",buffer, pNum);

            shmPtr->flag[pNum] = want_in; // Raise my flag
            j = shmPtr->turn; // Set local variable
            while ( j != pNum )                                            //while j != the process' constant
                j = ( shmPtr->flag[j] != idle ) ? shmPtr->turn : ( j + 1 ) % n;         //if the current process whose turn it is idle set j to (j+1) %n

            // Declare intention to enter critical section

            shmPtr->flag[pNum] = in_cs;

            // Check that no one else is in critical section

            for ( j = 0; j < n; j++ ){
                if ( ( j != pNum ) && ( shmPtr->flag[j] == in_cs ) ){
                    break;
                }
            }
        } while (( j < n ) || ( shmPtr->turn != pNum && shmPtr->flag[shmPtr->turn] != idle ));

        // Assign turn to self and enter critical section

        shmPtr->turn = pNum;      

		//Get's the time and outputs it when the  process is entering CS
		//reference:  https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
		
		time_t rawtime1;
		struct tm *info1;
		char buffer1[80];
		time( &rawtime1 );
		info1 = localtime( &rawtime1 );
		strftime(buffer1,80,"%x - %I:%M:%S%p", info1);
		
        fprintf(stderr, "\t| %s | \t process: %d\t | BEGIN Critical Section |\n", buffer1, pNum);

        //critical_section
        srand(time(NULL));
        int rN = rand()%3;
        sleep(rN);

        FILE * filePtr;
		

        if(isPalindrome(possiblePalindrome, 0, strlen(possiblePalindrome) -1)){
            filePtr = fopen("palin.out","a");

        }
        else{
            filePtr = fopen("nopalin.out","a");
        }
        fprintf(filePtr, "%ld %d %s\n", (long)getpid(), thisPalindromeNumber-n+1, possiblePalindrome);
        fclose(filePtr);

  
        rN = rand()%3;
        sleep(rN);
		
		//Get's the time and outputs it when the  process is leaving CS
		//reference:  https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm

        time_t rawtime2;
		struct tm *info2;
		char buffer2[80];
		time( &rawtime2 );
		info2 = localtime( &rawtime2 );
		strftime(buffer2,80,"%x - %I:%M:%S%p", info2);
        fprintf(stderr, "\t| %s | \t process: %d\t | LEAVE Critical Section |\n", buffer2, pNum);
	
      
        j = (shmPtr->turn + 1) % n;
        while (shmPtr->flag[j] == idle)
            j = (j + 1) % n;

        shmPtr->turn = j;
        shmPtr->flag[pNum] = idle;
        
    }
    
    shmdt(shmPtr);
    return 0;
}

	//function that checks if the string is a palindrome
	//returns 1 or 2; 1= true & 2 = false
	//ref:  http://www.geeksforgeeks.org/c-program-check-given-string-palindrome/

/* const int isPalindrome(char *str){
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
 */
 
 
 /*
 * Function to check whether a string is palindrome or not
 */
 int isPalindrome(char *inputString, int leftIndex, int rightIndex){
     /* Input Validation */
     if(NULL == inputString || leftIndex < 0 || rightIndex < 0){
         printf("Invalid Input");
         return 0;
     }
     /* Recursion termination condition */
     if(leftIndex >= rightIndex)
         return 1;
     if(inputString[leftIndex] == inputString[rightIndex]){
         return isPalindrome(inputString, leftIndex + 1, rightIndex - 1);
     }
     return 0;
 }

//detaches shared memory 
void exitfunc(int sig){	
    shmdt(shmPtr);
    exit(1);
}

void exitfuncCtrlC(int sig){

    fprintf( stderr, "Child Process: %ld  | Ctrl + C\n", (long)getpid());
    shmdt(shmPtr);
    exit(1);
}

void printTime(){
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "%s", asctime (timeinfo) );
}


