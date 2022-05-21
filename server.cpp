#include "Stack.hpp"
stack_pointer stackPointer;
void file(){
    //The  open()  system  call opens the file specified by pathname
    fd = open("file.txt", O_WRONLY | O_CREAT);
    if (fd==-1)// If -1 is returned, no files shall be created or modified.(https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html)
    {
        perror("ERROR: open the file");
    }
    memset(&lock, 0, sizeof(lock));
}
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
void *TREAD(void *temp){
    int *current_fd = (int *) temp;
    int new_fd = *current_fd;
    char input[MAXDATASIZE] = {0};//Initialize the array with 0 and not with "junk"
    int numbytes;

    while (1){
        bzero(input, MAXDATASIZE);
        if ((numbytes = recv(new_fd, input, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }
        input[numbytes] = '\0';
        printf(input);
        if (!strncmp(input, "EXIT", 4))
        {
            break;
        }
        else if (!(strncmp(input, "PUSH", 4)))
        {
            PUSH_stack(stackPointer, &input[5]);
            bzero(input, MAXDATASIZE);
        }
        else if (!strncmp(input, "POP", 3))
        {
            POP_stack(stackPointer);
        }
        else if (!strncmp(input, "TOP", 3))
        {
            TOP_stack(stackPointer, new_fd);
        }

    }
    close(new_fd);
    close (fd);
    return NULL;
}

int main(void)
{
    file();
    //DESCRIPTION: mmap()  creates a new mapping in the virtual address space of the calling process.
    //             The starting address for the new mapping is specified  in addr.
    stackPointer = (stack_pointer)mmap(NULL, sizeof(stackPointer), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    while (1){ // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork())
        { // this is the child process
            close(sockfd); // child doesn't need the listener
            TREAD(&new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }
    munmap(stackPointer, sizeof(Stack));
    return 0;
}