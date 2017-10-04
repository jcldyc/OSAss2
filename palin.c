#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

//const int isPalindrome(char *inputString, int leftIndex, int rightIndex);
const int isPalindrome(char * palindromeString);
void printTime();
void ctrlPlusC(int sig);
void exitSignal(int sig);

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
	
    if (signal(SIGUSR1, exitSignal) == SIG_ERR) {
        printf("SIGUSR1 error\n");
        exit(1);
    }
    if (signal(SIGINT, ctrlPlusC) == SIG_ERR) {
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

            shmPtr->flag[pNum] = want_in;
            j = shmPtr->turn; 
            while ( j != pNum )                                            
                j = ( shmPtr->flag[j] != idle ) ? shmPtr->turn( j + 1 ) % n;      

            shmPtr->flag[pNum] = in_cs;

            for ( j = 0; j < n; j++ ){
                if ( ( j != pNum ) && ( shmPtr->flag[j] == in_cs ) ){
                    break;
                }
            }
        } while (( j < n ) || ( shmPtr->turn != pNum && shmPtr->flag[shmPtr->turn] != idle ));

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
		

        if(isPalindrome(possiblePalindrome)){
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

const int isPalindrome(char *str){
    // Start from leftmost and rightmost corners of str
	
	int length = strlen(str);
	for(int i=0;i<length;i++){
		if(isalnum(str[i])){
			str[i]=tolower(str[i]);
		}
		
	}
	
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

 
 
 
 
 
 
 /* const int isPalindrome(char * palindromeString){


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



} */
 
 

//detaches shared memory 
void exitSignal(int sig){	
    shmdt(shmPtr);
    exit(1);
}

void ctrlPlusC(int sig){

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


