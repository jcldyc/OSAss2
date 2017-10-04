#include <stdio.h>
#include <string.h>

void isPalindrome(char str[]);

int main(int argc, char *argv[]){
	if(argc != 2){
		fprint("no following word");
		exit(1);
	}
	
	isPalindrome(argv[1]);
}
	
	// A function to check if a string str is palindrome
void isPalindrome(char str[])
{
    // Start from leftmost and rightmost corners of str
    int l = 0;
    int h = strlen(str) - 1;
 
    // Keep comparing characters while they are same
    while (h > l)
    {
        if (str[l++] != str[h--])
        {
            printf("%s is Not Palindromen", str);
            return;
        }
    }
    printf("%s is palindromen", str);
}
	
	
}