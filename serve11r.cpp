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
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include "stack.hpp"

//pmyStack stack1;
stackptr ss;
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
void *myThread(void *arg)
{
    char input[1024] = {0};
    int new_fd = *(int *)arg;
    int numbytes;
    while (strcmp(input, "EXIT"))
    {
        char str[1024];
        bzero(str, 1024);
        if (strncmp(input, "PUSH", 4) == 0)
        {
            for (int i = 5, j = 0; i < strlen(input); i++, j++)
            {
                str[j] = input[i];
            }
            push(str, ss);
            // printS(stack1);
        }
        else if (strncmp(input, "POP", 3) == 0)
        {
            pop(ss);
        }
        else if (strncmp(input, "TOP", 3) == 0)
        {
            top(ss, new_fd);
            // printS(stack1);
        }
        bzero(input, 1024);
        if ((numbytes = recv(new_fd, input, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }
        if (!numbytes)
        {
            printf("client disconnect\n");
            close(new_fd);
            close (fd);
            return NULL;
        }

        input[numbytes] = '\0';
        printf("server received: ");
        for (int i = 0; i < strlen(input); i++)
        {
            printf("%c", input[i]);
        }
        printf("\n");
    }
    printf("good bye\n");
    close(new_fd);
    close (fd);
    return NULL;
}

int main(void)
{
    createFile();
    ss = (stackptr)mmap(0, 2000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
    ss->data[0] = '\0';
    ss->top = 0;
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    // pthread_mutex_init(&mutex,NULL);

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
    pthread_t tid[CLIENT_NUMBER];
    int i = 0;
    while (1)
    { // main accept() loop
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
            i++;
            close(sockfd); // child doesn't need the listener
            myThread(&new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }

    return 0;
}