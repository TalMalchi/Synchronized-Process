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

using namespace std;
#include "stack.hpp"

#define MAXDATASIZE 100

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

// create new stack
stackptr ss;

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

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

int main(void)
{
	fileBlock();
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;

	ss = (stackptr)mmap(NULL, 1024 * 2048, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (ss == MAP_FAILED)
	{
		cout << "Memory allocation failed" << endl;
		perror("mmap");
		exit(1);
	};
	strcpy(ss[ss->top_counter + 1].arr, "NULL");

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

	while (true)
	{
		sin_size = sizeof their_addr;
		// get new connection for Client
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

		/*
		Each client connect to the server with new Process.
		This part of the code recvie message from the Client, and deals with it.
		In each action -PUSH, POP, TOP- we used "fcntl" to lock (and unlock at the end)
		the critical section in the code.
		In this way we ensure that one client can "chage" the stack each time.
		*/

		if (!fork())
		{				   // this is the child process
			close(sockfd); // child doesn't need the listener

			while (true)
			{ // child doesn't need the listener
				char buffer[1024];
				int numbytes;

				// struct flock fl;
				string line, cmd, data;
				try
				{

					memset(buffer, 0, 1024);
					// get msg from the Client
					if ((numbytes = recv(new_fd, buffer, sizeof(buffer), 0)) == -1)
					{
						cout << "fail in recieve" << endl;
						perror("recv");
						exit(1);

						buffer[numbytes] = '\0';
						printf("Server: received '%s'\n", buffer);
					}
					line = buffer;
					cmd = line.substr(0, line.find_first_of(" ")); // save the action

					if (cmd.size() < line.size())
					{
						// rest line is data for spesific cmd
						data = line.substr(line.find_first_of(" ") + 1);
					}
					// push Client's data to the stack
					if (cmd == "PUSH")
					{

						cout << "Data to Push: " << data << endl;
						cout << "Inside Push Server.cpp" << endl;

						push(data, ss);
					}
					// pop the last string from the stack
					else if (cmd == "POP")
					{
						cout << "Inside Pop Server.cpp" << endl;

						if (ss->size == 0)
						{
							cout << "Error- Stack is empty!" << endl;
						}
						else if (ss->size != 0)
						{ // check that the stack doesnt empty

							pop(ss);
						}
					}
					// Send to the CLient the last data on the stack
					else if (cmd == "TOP")
					{
						string empty_msg = "Error- Stack is empty!";
						// fl.l_type = F_WRLCK;
						// fcntl(fd, F_GETLK, &fl);
						cout << "Inside Top Server.cpp" << endl;

						if (ss->size == 0)
						{ // The client will get a msg if the stack is empty
							cout << empty_msg << endl;
							cout << "stack is empty" << endl;
							try
							{
								// send to Client empty_message
								if (send(new_fd, empty_msg.c_str(), empty_msg.size() + 1, 0) == -1)
								{
									perror("send");
									exit(1);
								}
								printf("send-Empty-MSG-to-Client\n");
								/// UNLOCK
								fl.l_type = F_UNLCK;
								fcntl(fd, F_GETLK, &fl);
							}
							catch (exception &e)
							{
								cout << "exception: Can't send message to Client" << endl;
							}
						}
						else if (ss->size != 0)
						{
							cout << "stack isnt empty" << endl;
							string top_msg;
							top_msg = top(ss);

							printf("TOP_MSG: '%s'\n", top_msg.c_str());
							try
							{
								// send to the Client the data on top of the stack
								if (send(new_fd, top_msg.c_str(), top_msg.size() + 1, 0) == -1)
								{
									perror("send");
									exit(1);
								}
								printf("send-top-to-client\n");
							}
							catch (exception &e)
							{
								cout << "exception: Can't send message to Client" << endl;
							}
						}
					}
				}
				catch (exception &e)
				{
					cout << "exception: didnt recieve msg" << endl;
				}
			}
			close(new_fd);
			close(fd);
			exit(0);
		}
		close(new_fd);
	}
	return 0; // parent doesn't need this
}