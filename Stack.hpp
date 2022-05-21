#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1024
//Credits:
//https://voyager.deanza.edu/~perry/lock.html,
// https://www.informit.com/articles/article.aspx?p=23618&seqNum=4
//https://www.youtube.com/watch?v=0wrdcxkgujc&ab_channel=ShriramVasudevan
int fd;
struct flock lock;
char endWord='\0';

typedef struct Stack
{
    int size;
    char data[MAXDATASIZE];
}*stack_pointer;


void PUSH_stack(stack_pointer stack, char *str){
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    for (int i = 0 ; i < strlen(str); i++){ stack->data[(stack->size)++ + 1] = str[i];}
    stack->data[(stack->size)++ + 1] = endWord; //adding '\0' for mark input termination
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
}
void POP_stack(stack_pointer stack){
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    if (stack->data[stack->size - 1] == NULL) {
        perror("ERROR: Stack is empty, thus cannot pop!");
    }
    stack->size-=1;
    int counter_pop=0;
    while (stack->data[stack->size - counter_pop] != endWord){counter_pop++;} //count how many chars we need to delete
    stack->size-=counter_pop;
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
}

void* TOP_stack(stack_pointer stack, int new_fd){
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    int lastWord= stack->size - 1;// removing '\0'
    char buffer[1024] = {0}; //Initialize the array with 0 and not with "junk"
    if (stack->data[stack->size - 1] == NULL) {
        perror("ERROR: Stack is empty, thus cannot top!");
    }
    strcat(buffer, "OUTPUT: ");// The strcat() function concatenates string2 to string1 and ends the resulting string with the null character
    int i = 8;
    while (stack->data[lastWord] != endWord){ lastWord-=1;} //A loop that will count the beginning of the last buffer
    while(stack->data[lastWord + 1] != endWord){ buffer[i++]=stack->data[(lastWord)++ + 1];} //Add the last input to the BUFFER
    if (send(new_fd, buffer, strlen(buffer), 0) == -1)    //print ths top of the stack by sending
    {
        perror("send");
    }
    lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &lock);
    return NULL;
}

