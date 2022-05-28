/*
** client.c -- a stream socket client demo
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <ostream>
using namespace std;

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

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

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2)
	{
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			  s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// send message to server
	char buffer[1024];
	string userInput;
	string cmd;

	do
	{
		// Gets cmd from the user and spilt it from the data (if we have some)
		getline(cin, userInput);
		cmd = userInput.substr(0, userInput.find_first_of(" "));
		// Each action we send to the Server
		if (cmd == "PUSH")
		{
			int sendRes = send(sockfd, userInput.c_str(), userInput.size() + 1, 0);
			printf("UserINPUT '%s'\n", userInput.c_str());
			if (sendRes == -1)
			{
				cout << "Could not send to server! Whoops!\r\n";
				continue;
			}
		}
		if (cmd == "POP")
		{
			int sendRes = send(sockfd, userInput.c_str(), userInput.size() + 1, 0);
			printf("UserINPUT '%s'\n", userInput.c_str());
			if (sendRes == -1)
			{
				cout << "Could not send to server! Whoops!\r\n";
				continue;
			}
		}

		if (cmd == "TOP")
		{
			int sendRes = send(sockfd, userInput.c_str(), userInput.size() + 1, 0);
			printf("UserINPUT '%s'\n", userInput.c_str());
			if (sendRes == -1)
			{
				cout << "Could not send to server! Whoops!\r\n";
				continue;
			}
			// server response, we get the TOP data of the stack
			try
			{

				memset(buffer, 0, 1024);
				cout << "Try to recieve TOP_MSG from server" << endl;
				if ((numbytes = recv(sockfd, buffer, sizeof(buffer), 0)) == -1)
				{
					cout << "fail in recieve" << endl;
					perror("recv");
					exit(1);
				}
				printf("Server: TOP_MSG: '%s'\n", buffer);
				printf("OUTPUT: '%s'\n", buffer);
			}
			catch (exception &e)
			{
				cout << "exception: didnt recieve msg" << endl;
			}
		}

	} while (true);

	printf("client: received '%s'\n", buf);

	close(sockfd);

	return 0;
}
